/**
 * @file NexDisplay.h
 * @brief Ponto de entrada único para o display Nextion.
 *
 * @details
 * `NexDisplay` centraliza toda a inicialização e o processamento
 * do loop. Em vez de chamar funções globais espalhadas, você usa
 * um único objeto com uma API coesa.
 *
 * ### Comparação: antes × depois
 *
 * **Antes (ITEADLIB original):**
 * @code
 * // Funções globais espalhadas
 * nexInit(115200, 16, 17, &Serial2);
 *
 * NexTouch *lista[] = { &btn0, &btn1, NULL };
 *
 * void loop() {
 *     nexLoop(lista);
 * }
 * @endcode
 *
 * **Depois (NextionUI):**
 * @code
 * NexDisplay display;
 * NexBotao btn0(0, 2, "b0");
 * NexBotao btn1(0, 3, "b1");
 *
 * void setup() {
 *     display.begin(Serial2, 115200, 16, 17);
 *     display.escutar(btn0);
 *     display.escutar(btn1);
 * }
 *
 * void loop() {
 *     display.atualizar();  // tudo em um lugar
 * }
 * @endcode
 *
 * ### Navegação entre páginas
 *
 * @code
 * display.irPara("p1_dash");   // por nome
 * display.irPara(1);            // por índice
 *
 * uint8_t pg = display.paginaAtual();
 * @endcode
 *
 * ### Callback de mudança de página
 *
 * @code
 * display.aoMudarPagina([](uint8_t pagina) {
 *     Serial.println("Mudou para: " + String(pagina));
 * });
 * @endcode
 *
 * ### Comandos diretos
 *
 * @code
 * display.cmd("dim=80");              // brilho 80%
 * display.cmdf("dim=%d", brilho);     // printf-style
 * display.brilho(80);                 // atalho
 * display.repouso(300);               // repouso após 300s
 * @endcode
 *
 * @author  professorThiago
 * @version 1.0.0
 * @license MIT
 */

#ifndef __NEXDISPLAY_H__
#define __NEXDISPLAY_H__

#include "NexComponentes.h"

/** @brief Tipo de callback para mudança de página. */
typedef void (*CbPagina)(uint8_t pagina);

/**
 * @brief Gerenciador principal do display Nextion.
 *
 * @details
 * Encapsula inicialização, lista de escuta de toque, processamento
 * do loop e navegação de páginas.
 */
class NexDisplay {
public:
    // -----------------------------------------------------------------------
    // Inicialização
    // -----------------------------------------------------------------------

    /**
     * @brief Inicializa o display Nextion.
     *
     * @param serial   Porta serial a usar (ex: Serial1, Serial2).
     * @param baudrate Velocidade. Padrão: 115200.
     * @param pinoRX   Pino RX do ESP32. -1 = usar padrão da porta.
     * @param pinoTX   Pino TX do ESP32. -1 = usar padrão da porta.
     *
     * @return true  se inicializado com sucesso.
     *
     * @par Exemplo
     * @code
     * NexDisplay display;
     *
     * void setup() {
     *     display.begin(Serial2, 115200, 16, 17);
     * }
     * @endcode
     */
    bool begin(HardwareSerial& serial,
               uint32_t baudrate = 115200,
               int8_t pinoRX    = -1,
               int8_t pinoTX    = -1);

    /**
     * @brief Processa eventos recebidos do display (toque, mudança de página).
     *
     * **Deve ser chamado em todo `loop()`.**
     *
     * @par Exemplo
     * @code
     * void loop() {
     *     display.atualizar();
     *     // resto do código
     * }
     * @endcode
     */
    void atualizar();

    // -----------------------------------------------------------------------
    // Lista de escuta de toque
    // -----------------------------------------------------------------------

    /**
     * @brief Registra um componente para receber eventos de toque.
     *
     * @details
     * O componente passado começa a ter seus callbacks disparados
     * automaticamente pelo `atualizar()`.
     *
     * @param componente  Referência para o componente a escutar.
     *
     * @return true  se registrado com sucesso.
     * @return false se a lista estiver cheia (NEX_MAX_COMPONENTES).
     *
     * @par Exemplo
     * @code
     * display.escutar(btnLigar);
     * display.escutar(btnDesligar);
     * display.escutar(sliderVolume);
     * @endcode
     */
    bool escutar(NexToque& componente);

    /**
     * @brief Remove todos os componentes da lista de escuta.
     */
    void limparEscuta();

    // -----------------------------------------------------------------------
    // Navegação
    // -----------------------------------------------------------------------

    /**
     * @brief Navega para uma página pelo nome.
     *
     * @param nome  Nome da página no Nextion Editor (ex: "p1_dash").
     *
     * @par Exemplo
     * @code
     * display.irPara("p2_ac");
     * @endcode
     */
    void irPara(const char* nome);

    /**
     * @brief Navega para uma página pelo índice.
     *
     * @param indice  Índice da página (0 = primeira).
     *
     * @par Exemplo
     * @code
     * display.irPara(1);   // segunda página
     * @endcode
     */
    void irPara(uint8_t indice);

    /**
     * @brief Retorna o índice da página atualmente exibida.
     *
     * @return Índice da página atual (atualizado pelos eventos do Nextion).
     */
    uint8_t paginaAtual() const { return _paginaAtual; }

    // -----------------------------------------------------------------------
    // Comandos diretos
    // -----------------------------------------------------------------------

    /**
     * @brief Envia um comando bruto ao Nextion.
     *
     * @param c  Comando (sem o terminador 0xFF).
     *
     * @par Exemplo
     * @code
     * display.cmd("dim=80");
     * display.cmd("sleep=1");
     * @endcode
     */
    void cmd(const char* c)  { nexEnviarCmd(c); }

    /**
     * @brief Envia um comando formatado (printf-style) ao Nextion.
     *
     * @par Exemplo
     * @code
     * display.cmdf("dim=%d", brilho);
     * display.cmdf("t0.txt=\"%d°C\"", temperatura);
     * @endcode
     */
    void cmdf(const char* fmt, ...);

    /**
     * @brief Define o brilho do display.
     *
     * @param pct  Porcentagem de brilho (0–100).
     *
     * @par Exemplo
     * @code
     * display.brilho(80);   // 80% de brilho
     * display.brilho(0);    // desliga a retroiluminação
     * @endcode
     */
    void brilho(uint8_t pct);

    /**
     * @brief Configura o repouso automático.
     *
     * @param timeoutS  Segundos sem toque para entrar em repouso.
     *                  0 = desativa o repouso automático.
     *
     * @par Exemplo
     * @code
     * display.repouso(300);   // repouso após 5 minutos
     * display.repouso(0);     // nunca entra em repouso
     * @endcode
     */
    void repouso(uint16_t timeoutS);

    // -----------------------------------------------------------------------
    // Callbacks
    // -----------------------------------------------------------------------

    /**
     * @brief Registra callback chamado quando a página muda.
     *
     * @param cb  Função `void cb(uint8_t pagina)`.
     *
     * @par Exemplo
     * @code
     * display.aoMudarPagina([](uint8_t pg) {
     *     if (pg == 5) {
     *         // Usuário entrou na tela de scan
     *         espnow.iniciarDescoberta();
     *     }
     * });
     * @endcode
     */
    void aoMudarPagina(CbPagina cb) { _cbPagina = cb; }

private:
    uint8_t    _paginaAtual = 0;
    CbPagina   _cbPagina    = nullptr;

    NexToque*  _lista[NEX_MAX_COMPONENTES + 1];
    uint8_t    _totalEscuta = 0;

    void _processarByte(uint8_t b);
    void _processarEvento(const uint8_t* buf, uint8_t tam);

    uint8_t  _rxBuf[16];
    uint8_t  _rxLen = 0;
};

#endif /* __NEXDISPLAY_H__ */
