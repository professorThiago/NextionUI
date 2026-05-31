/**
 * @file 04_NexDisplay.ino
 * @brief Sistema multipágina com NexDisplay e callbacks de página.
 *
 * ## O que este exemplo faz
 *
 * Demonstra um sistema com 3 páginas onde:
 * - `display.aoMudarPagina()` executa código específico por página
 * - `NexPagina` navega entre páginas pelo objeto
 * - `NexTimer` atualiza a tela em segundo plano
 * - `display.repouso()` ativa modo de repouso automático
 *
 * ## Estrutura do projeto Nextion
 *
 * ```
 * Página 0 (p0_sleep)  — Tela de repouso
 *   t_hora (Text, id=1)       — "14:32"
 *   t_data (Text, id=2)       — "Sex, 23/05/2025"
 *
 * Página 1 (p1_dash)   — Dashboard
 *   t_temp (Text, id=1)       — temperatura
 *   j_prog (ProgressBar, id=2)— barra de algum processo
 *   btn_config (Button, id=3) — vai para configurações
 *   tm_refresh (Timer, id=4)  — timer 2s para atualizar
 *
 * Página 2 (p2_config) — Configurações
 *   btn_voltar (Button, id=1) — volta para dashboard
 *   sl_brilho  (Slider, id=2) — ajusta brilho
 * ```
 *
 * @author  professorThiago
 * @version 1.0.0
 */

#include <NextionUI.h>

// ---------------------------------------------------------------------------
// Páginas
// ---------------------------------------------------------------------------

NexPagina p0Repouso(0, 0, "p0_sleep");
NexPagina p1Dash   (1, 0, "p1_dash");
NexPagina p2Config (2, 0, "p2_config");

// ---------------------------------------------------------------------------
// Componentes — página 0
// ---------------------------------------------------------------------------

NexTexto  tHora  (0, 1, "t_hora");
NexTexto  tData  (0, 2, "t_data");

// ---------------------------------------------------------------------------
// Componentes — página 1
// ---------------------------------------------------------------------------

NexTexto         tTemp    (1, 1, "t_temp");
NexBarraProgresso jProg   (1, 2, "j_prog");
NexBotao         btnConfig(1, 3, "btn_config");
NexTimer         tmRefresh(1, 4, "tm_refresh");

// ---------------------------------------------------------------------------
// Componentes — página 2
// ---------------------------------------------------------------------------

NexBotao  btnVoltar(2, 1, "btn_voltar");
NexSlider slBrilho (2, 2, "sl_brilho");

// ---------------------------------------------------------------------------
// Display
// ---------------------------------------------------------------------------

NexDisplay display;

// ---------------------------------------------------------------------------
// Dados simulados
// ---------------------------------------------------------------------------

float    temperatura = 23.5f;
uint32_t progresso   = 0;

// ---------------------------------------------------------------------------
// Funções de atualização por página
// ---------------------------------------------------------------------------

void atualizarPagina0() {
    // Simulação de hora/data (em produção: usar NTP + getLocalTime)
    static uint32_t tSec = 0;
    static uint8_t  s = 0, m = 0, h = 10;
    if (millis() - tSec >= 1000) {
        tSec = millis();
        if (++s >= 60) { s = 0; if (++m >= 60) { m = 0; h++; } }
    }
    tHora.setf("txt", "%02d:%02d", h, m);
    tData.texto("Sex, 23/05/2025");
}

void atualizarPagina1() {
    // Simula variação de temperatura
    temperatura += (random(-5, 6)) * 0.1f;
    if (temperatura < 18) temperatura = 18;
    if (temperatura > 35) temperatura = 35;

    // Cor muda conforme temperatura
    uint32_t cor;
    if      (temperatura < 22) cor = RGB565(0, 80, 200);   // azul = frio
    else if (temperatura < 28) cor = RGB565(0, 180, 0);    // verde = normal
    else                        cor = RGB565(220, 60, 0);   // laranja = quente

    tTemp.setf("txt", "%.1f°C", temperatura)
         .corTexto(cor);

    // Barra de progresso circula de 0 a 100
    progresso = (progresso + 2) % 101;
    jProg.valor(progresso);
}

// ---------------------------------------------------------------------------
// setup()
// ---------------------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    delay(500);

    display.begin(Serial2, 115200, 16, 17);
    display.brilho(80);
    display.repouso(300);  // repouso após 5 min sem toque

    // ── Registrar todos os componentes com toque ───────────────────────
    display.escutar(p0Repouso);
    display.escutar(p1Dash);
    display.escutar(p2Config);
    display.escutar(btnConfig);
    display.escutar(btnVoltar);
    display.escutar(slBrilho);
    display.escutar(tmRefresh);

    // ── Callbacks de navegação ─────────────────────────────────────────
    btnConfig.aoSoltar([]() { p2Config.mostrar(); });
    btnVoltar.aoSoltar([]() { p1Dash.mostrar();   });

    // ── Slider de brilho ──────────────────────────────────────────────
    slBrilho.minimo(10).maximo(100).valor(80);
    slBrilho.aoSoltar([]() {
        uint32_t v;
        if (slBrilho.valor(v)) display.brilho((uint8_t)v);
    });

    // ── Timer — atualiza dados a cada 2s enquanto na página 1 ─────────
    tmRefresh.ciclo(2000);
    tmRefresh.iniciar();
    tmRefresh.aoDisparar([]() { atualizarPagina1(); });

    // ── Callback de mudança de página ──────────────────────────────────
    display.aoMudarPagina([](uint8_t pg) {
        Serial.println("Pagina: " + String(pg));
        switch (pg) {
            case 0:
                // Ao entrar na tela de repouso: para o timer
                tmRefresh.parar();
                break;
            case 1:
                // Ao entrar no dashboard: (re)inicia o timer
                tmRefresh.iniciar();
                atualizarPagina1();
                break;
            case 2:
                // Ao entrar em config: para o timer para economizar
                tmRefresh.parar();
                slBrilho.valor(80);
                break;
        }
    });

    // ── Toque em qualquer lugar na tela de repouso → vai para dashboard
    p0Repouso.aoPressionar([]() { p1Dash.mostrar(); });

    // Inicia na página de repouso
    p0Repouso.mostrar();
    atualizarPagina0();
}

// ---------------------------------------------------------------------------
// loop()
// ---------------------------------------------------------------------------

void loop() {
    display.atualizar();

    // Atualiza relógio na tela de repouso se estiver nela
    static uint32_t tRel = 0;
    if (display.paginaAtual() == 0 && millis() - tRel >= 1000) {
        tRel = millis();
        atualizarPagina0();
    }
}
