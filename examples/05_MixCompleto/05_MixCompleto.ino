/**
 * @file 05_MixCompleto.ino
 * @brief Demonstração completa: timer, gauge, grafico, RTC, GPIO, texto deslizante.
 *
 * ## O que este exemplo faz
 *
 * Mostra os componentes menos comuns da biblioteca em um único exemplo:
 *
 * - `NexTimer`            — atualiza dados periodicamente
 * - `NexGauge`            — mostra temperatura como velocímetro
 * - `NexGrafico`          — plota leitura do ADC em tempo real
 * - `NexRtc`              — sincroniza e lê o RTC do Nextion
 * - `NexGpio`             — pisca um LED conectado ao GPIO do Nextion
 * - `NexTextoDeslizante`  — exibe aviso em rolagem
 * - `NexCheckbox`         — habilita/desabilita o gráfico
 * - `NexVariavel`         — armazena estado no próprio display
 *
 * ## Componentes no Nextion Editor
 *
 * | Tipo          | objname   | Pg | ID |
 * |---------------|-----------|----|----|
 * | Timer         | tm_dados  | 0  | 1  |
 * | Gauge         | z_temp    | 0  | 2  |
 * | Waveform      | s_adc     | 0  | 3  |
 * | Scrolltext    | x_aviso   | 0  | 4  |
 * | Checkbox      | cb_grafico| 0  | 5  |
 * | Variable      | va_estado | 0  | 6  |
 * | Text          | t_rtc     | 0  | 7  |
 *
 * @author  professorThiago
 * @version 1.0.0
 */

#include <NextionUI.h>

// ---------------------------------------------------------------------------
// Objetos globais
// ---------------------------------------------------------------------------

NexDisplay           display;
NexTimer             tmDados   (0, 1, "tm_dados");
NexGauge             zTemp     (0, 2, "z_temp");
NexGrafico           sAdc      (0, 3, "s_adc");
NexTextoDeslizante   xAviso    (0, 4, "x_aviso");
NexCheckbox          cbGrafico (0, 5, "cb_grafico");
NexVariavel          vaEstado  (0, 6, "va_estado");
NexTexto             tRtc      (0, 7, "t_rtc");

NexRtc  rtc;
NexGpio gpio;

bool grafAtivo = true;

// ---------------------------------------------------------------------------
// setup()
// ---------------------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    delay(500);

    display.begin(Serial2, 115200, 16, 17);
    display.brilho(80);

    // Registra componentes com toque
    display.escutar(tmDados);
    display.escutar(cbGrafico);

    // ── NexRtc — sincronizar hora (simulado; em produção: usar NTP) ───────
    rtc.escrever(2025, 5, 23, 14, 32, 0);
    Serial.println("RTC sincronizado.");

    // ── NexGpio — configura GPIO 0 do Nextion como saída ────────────────
    gpio.modoSaida(0);

    // ── NexTextoDeslizante — aviso em rolagem ────────────────────────────
    xAviso.texto("Reunião pedagógica às 14h na sala de professores")
          .ciclo(150)
          .iniciar();

    // ── NexCheckbox — habilita/desabilita gráfico ────────────────────────
    cbGrafico.marcado(true);
    cbGrafico.aoSoltar([]() {
        bool estado;
        cbGrafico.marcado(estado);
        grafAtivo = estado;
        sAdc.visivel(grafAtivo);
        Serial.println("Grafico: " + String(grafAtivo ? "ON" : "OFF"));
    });

    // ── NexVariavel — salva estado no próprio display ────────────────────
    vaEstado.valor(1);  // 1 = sistema ativo

    // ── NexTimer — atualiza dados a cada 500ms ──────────────────────────
    tmDados.ciclo(500).iniciar();
    tmDados.aoDisparar([]() {
        // Leitura simulada de temperatura (18-35°C)
        static float temp = 25.0f;
        temp += (random(-3, 4)) * 0.2f;
        temp = constrain(temp, 18.0f, 35.0f);

        // Gauge: mapeia 18-35°C para 0-270 graus
        uint32_t angulo = (uint32_t)((temp - 18.0f) / 17.0f * 270.0f);
        zTemp.valor(angulo);

        // Gráfico ADC
        if (grafAtivo) {
            uint8_t pontoAdc = (uint8_t)(analogRead(34) >> 4); // 12bit → 8bit
            sAdc.adicionarPonto(0, pontoAdc);
        }

        // Pisca GPIO 0 do Nextion
        static bool ledState = false;
        gpio.escrever(0, ledState ? 1 : 0);
        ledState = !ledState;
    });

    // ── Ler RTC a cada 10s e atualizar texto ─────────────────────────────
    // (feito no loop() para não depender de outro timer)

    Serial.println("Pronto.");
}

// ---------------------------------------------------------------------------
// loop()
// ---------------------------------------------------------------------------

void loop() {
    display.atualizar();

    // Atualiza display do RTC a cada 1s
    static uint32_t tRtcUlt = 0;
    if (millis() - tRtcUlt >= 1000) {
        tRtcUlt = millis();

        uint32_t hora, minuto, segundo;
        if (rtc.lerHora(hora, minuto, segundo)) {
            tRtc.setf("txt", "%02lu:%02lu:%02lu", hora, minuto, segundo);
        }
    }
}
