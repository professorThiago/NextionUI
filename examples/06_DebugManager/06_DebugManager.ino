/**
 * @file 06_DebugManager.ino
 * @brief Integração com DebugManager para log detalhado da biblioteca.
 *
 * ## O que este exemplo faz
 *
 * Demonstra como ativar o debug da biblioteca NextionUI via DebugManager:
 *
 * 1. Inclua `DebugManager.h` **antes** de `NextionUI.h`.
 * 2. O debug da biblioteca passa automaticamente pelo DebugManager.
 * 3. Mensagens internas aparecem com prefixo `[Nextion]`.
 *
 * ## Comparação dos níveis de debug
 *
 * | Nível       | O que aparece da biblioteca NextionUI          |
 * |-------------|------------------------------------------------|
 * | DEBUG_ERRO  | Falhas de comunicação, timeouts                |
 * | DEBUG_INFO  | Inicialização, páginas                         |
 * | DEBUG_VERBOSE | Todos os envios, leituras, eventos de toque |
 *
 * ## Saída esperada no Serial Monitor
 *
 * ```
 * [INFO]  ESP32 — DebugManager v3.0.0
 * [INFO]  Nivel ativo: 4 - VERBOSE
 * [INFO]    [Nextion] Nextion inicializado.
 * [VERBOSE] [Nextion] Toque pag=0 comp=2 evt=1
 * [VERBOSE] [Nextion] Toque pag=0 comp=2 evt=0
 * [INFO]    Botão pressionado!
 * ```
 *
 * ## Sem DebugManager (apenas Serial)
 *
 * Se não quiser usar DebugManager, defina `NEX_DEBUG_SERIAL`:
 *
 * @code
 * #define NEX_DEBUG_SERIAL Serial    // ativa debug via Serial direto
 * #include <NextionUI.h>
 * @endcode
 *
 * @author  professorThiago
 * @version 1.0.0
 */

// ── IMPORTANTE: incluir DebugManager ANTES de NextionUI ──────────────────
#include <DebugManager.h>
#include <NextionUI.h>

// ---------------------------------------------------------------------------
// Objetos globais
// ---------------------------------------------------------------------------

NexDisplay display;
NexBotao   btn0  (0, 2, "b0");
NexTexto   tInfo (0, 3, "t_info");
NexNumero  nClk  (0, 4, "n_clk");

uint32_t totalCliques = 0;

// ---------------------------------------------------------------------------
// setup()
// ---------------------------------------------------------------------------

void setup() {
    // Nível VERBOSE para ver os eventos internos do Nextion
    // Conecte GPIO0 a GND antes de ligar para forçar DEBUG_TUDO
    configurarDebug(DEBUG_VERBOSE, 0);

    debugInfo("=== Exemplo 06: NextionUI + DebugManager ===");

    display.begin(Serial2, 115200, 16, 17);
    display.brilho(80);
    display.escutar(btn0);

    btn0.aoSoltar([]() {
        totalCliques++;
        nClk.valor(totalCliques);
        tInfo.setf("txt", "Cliques: %lu", totalCliques);
        debugInfo("Botao clicado. Total: " + String(totalCliques));
    });

    tInfo.texto("Clique no botao");
    nClk.valor(0);

    debugInfo("Setup concluido. Aguardando eventos...");

    // Demonstra os diferentes níveis de log
    debugErro  ("Exemplo de ERRO   — vermelho no monitor");
    debugAviso ("Exemplo de AVISO  — amarelo no monitor");
    debugInfo  ("Exemplo de INFO   — azul no monitor");
    debugVerbose("Exemplo de VERBOSE — cinza no monitor");
}

// ---------------------------------------------------------------------------
// loop()
// ---------------------------------------------------------------------------

void loop() {
    display.atualizar();
}
