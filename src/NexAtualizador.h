/**
 * @file NexAtualizador.h
 * @brief Atualização de firmware (.tft) do display Nextion via serial.
 *
 * @details
 * Implementa o protocolo de upload do Nextion, permitindo atualizar
 * a interface do display remotamente via ESP32, sem necessidade de
 * cartão SD ou conexão USB direta ao Nextion Editor.
 *
 * ### Fontes suportadas
 *
 * Aceita qualquer objeto `Stream` como fonte de dados:
 * - `HTTPClient::getStream()` — download HTTP/HTTPS direto do servidor
 * - `SD.open()` — leitura de cartão SD
 * - `SPIFFS.open()` — leitura de flash interna
 *
 * O upload é feito por streaming — não requer RAM para o arquivo inteiro.
 * Apenas um buffer de bloco (padrão: 4096 bytes) é necessário.
 *
 * ### Protocolo Nextion Upload
 *
 * 1. ESP32 envia comando `whmi-wri <tam>,<baud>,0` na baud atual.
 * 2. Nextion responde com `0x05` e troca para a baud de upload.
 * 3. ESP32 troca para a mesma baud e envia blocos de 4096 bytes.
 * 4. Nextion responde `0x05` após cada bloco.
 * 5. Após o último bloco, o Nextion reinicia automaticamente.
 *
 * ### Uso típico — download HTTP
 *
 * @code
 * #include <NexAtualizador.h>
 * #include <HTTPClient.h>
 *
 * NexAtualizador nexOta(Serial2);
 *
 * void atualizarTela(const char* url) {
 *     HTTPClient http;
 *     http.begin(url);
 *     int codigo = http.GET();
 *     if (codigo != 200) return;
 *
 *     uint32_t tamanho = http.getSize();
 *     Stream& fonte = http.getStream();
 *
 *     nexOta.aoProgresso([](uint8_t pct) {
 *         Serial.printf("Nextion: %d%%\n", pct);
 *     });
 *
 *     if (nexOta.atualizar(fonte, tamanho)) {
 *         Serial.println("Tela atualizada com sucesso!");
 *     }
 *
 *     http.end();
 * }
 * @endcode
 *
 * ### Uso típico — cartão SD
 *
 * @code
 * #include <NexAtualizador.h>
 * #include <SD.h>
 *
 * NexAtualizador nexOta(Serial2);
 *
 * void atualizarTelaSD() {
 *     File arquivo = SD.open("/interface.tft");
 *     if (!arquivo) return;
 *
 *     nexOta.atualizar(arquivo, arquivo.size());
 *     arquivo.close();
 * }
 * @endcode
 *
 * @note Durante o upload, o display exibe uma tela azul com barra de
 *       progresso nativa do Nextion. A comunicação normal (NexDisplay)
 *       fica indisponível até o Nextion reiniciar.
 *
 * @author  professorThiago (https://github.com/professorThiago)
 * @version 1.0.0
 * @date    2025
 * @license MIT
 */

#ifndef NEX_ATUALIZADOR_H
#define NEX_ATUALIZADOR_H

#include <Arduino.h>
#include <HardwareSerial.h>

// ---------------------------------------------------------------------------
// Integração com DebugManager (opcional)
// ---------------------------------------------------------------------------
#if __has_include("DebugManager.h")
  #include "DebugManager.h"
  #define _NEX_UP_TEM_DEBUG 1
#endif

#ifndef _NEX_UP_TEM_DEBUG
  #define debugInfo(x)
  #define debugErro(x)
  #define debugAviso(x)
  #define debugVerbose(x)
  #define debugTudo(x)
#endif

// ---------------------------------------------------------------------------
// Limites configuráveis — redefina ANTES do #include para personalizar
// ---------------------------------------------------------------------------

/** @brief Tamanho do bloco de envio em bytes. Nextion usa 4096. */
#ifndef NEX_UPLOAD_TAMANHO_BLOCO
  #define NEX_UPLOAD_TAMANHO_BLOCO  4096
#endif

/** @brief Tempo máximo de espera pela resposta 0x05 (ms). */
#ifndef NEX_UPLOAD_TIMEOUT_MS
  #define NEX_UPLOAD_TIMEOUT_MS  5000
#endif

/** @brief Baud rate padrão para upload. 115200 é o mais confiável. */
#ifndef NEX_UPLOAD_BAUD_PADRAO
  #define NEX_UPLOAD_BAUD_PADRAO  115200
#endif

/** @brief Byte de confirmação do Nextion. */
#define NEX_UPLOAD_ACK  0x05

// ---------------------------------------------------------------------------
// Códigos de erro
// ---------------------------------------------------------------------------

/**
 * @brief Códigos de erro do upload Nextion.
 */
enum class ErroNexUpload : int {
    NENHUM              =  0,
    SEM_RESPOSTA        = -1,  ///< Nextion não respondeu ao comando de upload
    ACK_INVALIDO        = -2,  ///< Resposta inesperada (não 0x05)
    LEITURA_FONTE       = -3,  ///< Erro ao ler dados da fonte (Stream)
    BLOCO_SEM_ACK       = -4,  ///< Bloco enviado mas sem confirmação
    TAMANHO_ZERO        = -5,  ///< Tamanho do arquivo é zero
    TIMEOUT_CONEXAO     = -6,  ///< Timeout aguardando conexão com display
};

// ---------------------------------------------------------------------------
// Tipos de callback
// ---------------------------------------------------------------------------

/** @brief Callback de progresso (0–100). */
typedef void (*CallbackProgressoNex)(uint8_t porcentagem);

/** @brief Callback de erro. */
typedef void (*CallbackErroNex)(ErroNexUpload codigo, const char* mensagem);

// =============================================================================
// NexAtualizador
// =============================================================================

/**
 * @brief Atualiza o firmware (.tft) do display Nextion via serial.
 *
 * Classe independente — não requer NexDisplay. Usa apenas a
 * HardwareSerial conectada ao Nextion.
 *
 * @par Exemplo completo
 * @code
 * NexAtualizador nexOta(Serial2);
 *
 * // Configuração (opcional — padrões funcionam para maioria dos casos)
 * nexOta.definirBaudOperacao(921600);  // baud atual do display
 * nexOta.definirBaudUpload(115200);    // baud para transferência
 *
 * // Callbacks
 * nexOta.aoProgresso([](uint8_t pct) {
 *     Serial.printf("Upload Nextion: %d%%\n", pct);
 * });
 *
 * nexOta.aoErro([](ErroNexUpload cod, const char* msg) {
 *     Serial.printf("Erro: %s\n", msg);
 * });
 *
 * // Atualizar de um Stream
 * nexOta.atualizar(fonte, tamanho);
 * @endcode
 */
class NexAtualizador {
public:
    // -----------------------------------------------------------------------
    // Construtor
    // -----------------------------------------------------------------------

    /**
     * @brief Construtor.
     *
     * @param serial  Porta serial conectada ao Nextion (ex: `Serial2`).
     */
    explicit NexAtualizador(HardwareSerial& serial);

    // -----------------------------------------------------------------------
    // Atualização
    // -----------------------------------------------------------------------

    /**
     * @brief Atualiza o display a partir de um Stream.
     *
     * Faz streaming da fonte para o Nextion em blocos de
     * NEX_UPLOAD_TAMANHO_BLOCO bytes. Não requer RAM para o arquivo
     * inteiro — ideal para download HTTP direto.
     *
     * @param fonte    Stream de leitura (HTTPClient, File, etc.).
     * @param tamanho  Tamanho total do arquivo .tft em bytes.
     * @return true    se o upload foi concluído com sucesso.
     *
     * @note O Nextion reinicia automaticamente após o upload.
     *       A comunicação normal (NexDisplay) só volta após
     *       reinicializar a serial.
     */
    bool atualizar(Stream& fonte, uint32_t tamanho);

    /**
     * @brief Atualiza o display a partir de um buffer na memória.
     *
     * Útil para arquivos pequenos já carregados na PSRAM.
     *
     * @param dados    Ponteiro para o conteúdo do .tft.
     * @param tamanho  Tamanho em bytes.
     * @return true    se bem-sucedido.
     */
    bool atualizar(const uint8_t* dados, uint32_t tamanho);

    // -----------------------------------------------------------------------
    // Configuração
    // -----------------------------------------------------------------------

    /**
     * @brief Define a baud rate atual de operação do display.
     *
     * Esta é a baud na qual o Nextion está rodando ANTES do upload.
     * O comando de upload é enviado nesta velocidade.
     *
     * @param baud  Baud rate atual (padrão: 9600 de fábrica do Nextion).
     *
     * @note Se você usa `bauds=921600` no Nextion Editor, defina 921600.
     */
    void definirBaudOperacao(uint32_t baud) { _baudOperacao = baud; }

    /**
     * @brief Define a baud rate para a transferência do upload.
     *
     * Após o comando de upload, o Nextion troca para esta baud.
     * 115200 é o mais confiável. Valores maiores são mais rápidos
     * mas podem ter erros em conexões longas.
     *
     * @param baud  Baud de upload (padrão: NEX_UPLOAD_BAUD_PADRAO).
     */
    void definirBaudUpload(uint32_t baud) { _baudUpload = baud; }

    /**
     * @brief Define o timeout de espera pela resposta do display.
     *
     * @param ms  Timeout em milissegundos (padrão: NEX_UPLOAD_TIMEOUT_MS).
     */
    void definirTempoLimite(uint32_t ms) { _tempoLimite = ms; }

    // -----------------------------------------------------------------------
    // Callbacks
    // -----------------------------------------------------------------------

    /**
     * @brief Callback de progresso do upload.
     * @param cb  `void cb(uint8_t porcentagem)` — valor de 0 a 100.
     */
    void aoProgresso(CallbackProgressoNex cb) { _cbProgresso = cb; }

    /**
     * @brief Callback de erro.
     * @param cb  `void cb(ErroNexUpload codigo, const char* mensagem)`.
     */
    void aoErro(CallbackErroNex cb) { _cbErro = cb; }

private:
    HardwareSerial& _serial;

    uint32_t _baudOperacao = 9600;
    uint32_t _baudUpload   = NEX_UPLOAD_BAUD_PADRAO;
    uint32_t _tempoLimite  = NEX_UPLOAD_TIMEOUT_MS;

    CallbackProgressoNex _cbProgresso = nullptr;
    CallbackErroNex      _cbErro      = nullptr;

    bool    _iniciarProtocolo(uint32_t tamanho);
    bool    _enviarBloco(const uint8_t* dados, size_t tamanho);
    bool    _aguardarAck();
    void    _limparBuffer();
    void    _enviarComandoNex(const char* cmd);
    void    _reportarErro(ErroNexUpload codigo, const char* mensagem);
};

#endif // NEX_ATUALIZADOR_H
