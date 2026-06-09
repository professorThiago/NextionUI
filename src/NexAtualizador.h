/**
 * @file NexAtualizador.h
 * @brief Atualização de firmware (.tft) do display Nextion via serial.
 *
 * @details
 * Implementa o protocolo de upload do Nextion com auto-detecção de baud rate,
 * permitindo atualizar a interface do display remotamente via ESP32.
 *
 * ### Processo de upload (igual ao Nextion Editor)
 *
 * 1. **Auto-detecção** — testa bauds (9600, 115200, 19200, ...) até o display responder
 * 2. **Comando upload** — envia `whmi-wri <tam>,<baud>,0` na baud detectada
 * 3. **Troca de baud** — display e ESP32 trocam para a baud de upload
 * 4. **Transferência** — envia blocos de 4096 bytes com ACK (0x05) por bloco
 * 5. **Reinício** — Nextion reinicia automaticamente com a nova interface
 *
 * ### Uso típico
 *
 * @code
 * NexAtualizador nexOta(Serial2);
 *
 * // Não precisa saber a baud — auto-detecta!
 * nexOta.definirBaudUpload(115200);
 *
 * HTTPClient http;
 * http.begin(url);
 * http.GET();
 * nexOta.atualizar(*http.getStreamPtr(), http.getSize());
 * http.end();
 * @endcode
 *
 * @author  professorThiago (https://github.com/professorThiago)
 * @version 1.1.0
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
// Limites configuráveis
// ---------------------------------------------------------------------------

#ifndef NEX_UPLOAD_TAMANHO_BLOCO
  #define NEX_UPLOAD_TAMANHO_BLOCO  4096
#endif

#ifndef NEX_UPLOAD_TIMEOUT_MS
  #define NEX_UPLOAD_TIMEOUT_MS  5000
#endif

#ifndef NEX_UPLOAD_BAUD_PADRAO
  #define NEX_UPLOAD_BAUD_PADRAO  115200
#endif

#define NEX_UPLOAD_ACK  0x05

// ---------------------------------------------------------------------------
// Códigos de erro
// ---------------------------------------------------------------------------

enum class ErroNexUpload : int {
    NENHUM              =  0,
    SEM_RESPOSTA        = -1,
    ACK_INVALIDO        = -2,
    LEITURA_FONTE       = -3,
    BLOCO_SEM_ACK       = -4,
    TAMANHO_ZERO        = -5,
    TIMEOUT_CONEXAO     = -6,
};

// ---------------------------------------------------------------------------
// Tipos de callback
// ---------------------------------------------------------------------------

typedef void (*CallbackProgressoNex)(uint8_t porcentagem);
typedef void (*CallbackErroNex)(ErroNexUpload codigo, const char* mensagem);

// =============================================================================
// NexAtualizador
// =============================================================================

/**
 * @brief Atualiza o firmware (.tft) do display Nextion via serial.
 *
 * Detecta automaticamente a baud rate do display (como o Nextion Editor),
 * negocia a troca para a velocidade de upload e envia o arquivo por streaming.
 *
 * @par Exemplo
 * @code
 * NexAtualizador nexOta(Serial2);
 * nexOta.definirBaudUpload(115200);  // velocidade de transferência
 *
 * // Auto-detecta a baud do display, negocia, e envia
 * nexOta.atualizar(httpStream, tamanho);
 * @endcode
 */
class NexAtualizador {
public:
    /**
     * @brief Construtor.
     * @param serial  Porta serial conectada ao Nextion (ex: `Serial2`).
     */
    explicit NexAtualizador(HardwareSerial& serial);

    /**
     * @brief Atualiza o display a partir de um Stream (HTTP, SD, SPIFFS).
     *
     * Auto-detecta a baud do display, negocia upload, e envia por streaming.
     *
     * @param fonte    Stream de leitura.
     * @param tamanho  Tamanho total do arquivo .tft em bytes.
     * @return true    se o upload foi concluído com sucesso.
     */
    bool atualizar(Stream& fonte, uint32_t tamanho);

    /**
     * @brief Atualiza o display a partir de um buffer na memória.
     *
     * @param dados    Ponteiro para o conteúdo do .tft.
     * @param tamanho  Tamanho em bytes.
     * @return true    se bem-sucedido.
     */
    bool atualizar(const uint8_t* dados, uint32_t tamanho);

    /**
     * @brief Define a baud rate para a transferência do upload.
     *
     * O display negocia a troca após aceitar o comando de upload.
     * 115200 é o mais confiável. 921600 é mais rápido.
     *
     * @param baud  Baud de upload (padrão: NEX_UPLOAD_BAUD_PADRAO = 115200).
     */
    void definirBaudUpload(uint32_t baud) { _baudUpload = baud; }

    /**
     * @brief Define o timeout de espera pela resposta do display.
     * @param ms  Timeout em milissegundos (padrão: NEX_UPLOAD_TIMEOUT_MS).
     */
    void definirTempoLimite(uint32_t ms) { _tempoLimite = ms; }

    /**
     * @brief Retorna a baud rate detectada do display (após atualizar).
     * @return Baud detectada, ou 0 se ainda não detectou.
     */
    uint32_t baudDetectada() const { return _baudDetectada; }

    /** @brief Callback de progresso (0–100). */
    void aoProgresso(CallbackProgressoNex cb) { _cbProgresso = cb; }

    /** @brief Callback de erro. */
    void aoErro(CallbackErroNex cb) { _cbErro = cb; }

    // ── Métodos legados (compatibilidade) ─────────────────────────────
    /** @deprecated Use apenas definirBaudUpload(). A baud de operação é auto-detectada. */
    void definirBaudOperacao(uint32_t baud) { (void)baud; }

private:
    HardwareSerial& _serial;

    uint32_t _baudUpload    = NEX_UPLOAD_BAUD_PADRAO;
    uint32_t _baudDetectada = 0;
    uint32_t _tempoLimite   = NEX_UPLOAD_TIMEOUT_MS;

    CallbackProgressoNex _cbProgresso = nullptr;
    CallbackErroNex      _cbErro      = nullptr;

    bool    _detectarBaud();
    bool    _iniciarProtocolo(uint32_t tamanho);
    bool    _enviarBloco(const uint8_t* dados, size_t tamanho);
    bool    _aguardarAck();
    void    _limparBuffer();
    void    _enviarComandoNex(const char* cmd);
    void    _reportarErro(ErroNexUpload codigo, const char* mensagem);
};

#endif // NEX_ATUALIZADOR_H
