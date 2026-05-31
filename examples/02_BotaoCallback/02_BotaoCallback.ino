/**
 * @file 02_BotaoCallback.ino
 * @brief Registrar callbacks em botões usando lambdas.
 *
 * ## O que este exemplo faz
 *
 * Demonstra as duas formas de registrar callbacks em componentes com toque:
 *
 * 1. **Lambda simples** (recomendada) — sintaxe moderna e concisa.
 * 2. **Função global com ponteiro** (compatibilidade) — estilo da ITEADLIB.
 *
 * Também mostra:
 * - Como mudar cor do texto conforme o estado
 * - Como ler o estado de um botão de dois estados (toggle)
 *
 * ## Componentes necessários no Nextion Editor
 *
 * | Tipo          | objname   | Página | ID |
 * |---------------|-----------|--------|----|
 * | Button        | btn_ligar | 0      | 2  |
 * | Button        | btn_desl  | 0      | 3  |
 * | DualStateBtn  | bt_toggle | 0      | 4  |
 * | Text          | t_status  | 0      | 5  |
 *
 * @author  professorThiago
 * @version 1.0.0
 */

#include <NextionUI.h>

// ---------------------------------------------------------------------------
// Objetos globais
// ---------------------------------------------------------------------------

NexDisplay     display;
NexBotao       btnLigar (0, 2, "btn_ligar");
NexBotao       btnDesl  (0, 3, "btn_desl");
NexBotaoDuplo  btToggle (0, 4, "bt_toggle");
NexTexto       tStatus  (0, 5, "t_status");

bool estadoLigado = false;

// ---------------------------------------------------------------------------
// Função de callback estilo antigo (com ponteiro — para compatibilidade)
// ---------------------------------------------------------------------------

void cbBtnDesligar(void* ptr) {
    // ptr aponta para o próprio botão, se necessário
    estadoLigado = false;
    tStatus.texto("DESLIGADO")
           .corTexto(COR_VERMELHO)
           .corFundo(RGB565(50, 0, 0));
    Serial.println("Desligado via callback antigo");
}

// ---------------------------------------------------------------------------
// setup()
// ---------------------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    delay(500);

    display.begin(Serial2, 115200, 16, 17);
    display.brilho(80);

    // ── Registrar componentes para escuta de toque ──────────────────────
    display.escutar(btnLigar);
    display.escutar(btnDesl);
    display.escutar(btToggle);

    // ── Callbacks com lambda (recomendado) ──────────────────────────────

    // Callback ao SOLTAR o botão Ligar
    btnLigar.aoSoltar([]() {
        estadoLigado = true;
        tStatus.texto("LIGADO")
               .corTexto(COR_BRANCO)
               .corFundo(RGB565(0, 100, 0));
        Serial.println("Ligado!");
    });

    // Callback ao PRESSIONAR — feedback visual imediato
    btnLigar.aoPressionar([]() {
        // Escurece o botão ao pressionar
        btnLigar.corFundo(RGB565(0, 60, 120));
    });

    // ── Callback estilo antigo (com ponteiro) ────────────────────────────
    btnDesl.aoSoltar(cbBtnDesligar, nullptr);

    // ── Toggle — lê o estado ao soltar ──────────────────────────────────
    btToggle.aoSoltar([]() {
        uint32_t estado;
        if (btToggle.valor(estado)) {
            Serial.println("Toggle: " + String(estado ? "ON" : "OFF"));
            tStatus.setf("txt", "Toggle: %s", estado ? "ON" : "OFF");
        }
    });

    // ── Callback de mudança de página ────────────────────────────────────
    display.aoMudarPagina([](uint8_t pg) {
        Serial.println("Mudou para pagina " + String(pg));
    });

    // Estado inicial
    tStatus.texto("Aguardando...");
    Serial.println("Pronto. Toque nos botoes.");
}

// ---------------------------------------------------------------------------
// loop()
// ---------------------------------------------------------------------------

void loop() {
    display.atualizar();
}
