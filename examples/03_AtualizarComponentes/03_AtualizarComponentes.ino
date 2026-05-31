/**
 * @file 03_AtualizarComponentes.ino
 * @brief Demonstra os métodos de atualização de todos os tipos de componente.
 *
 * ## O que este exemplo faz
 *
 * Mostra todas as formas de atualizar componentes:
 *
 * 1. `texto()` e `setf()` para texto formatado
 * 2. `valor()` para números e sliders
 * 3. `set("atributo", valor)` para qualquer atributo
 * 4. `corFundo()` e `corTexto()` como atalhos
 * 5. `visivel()` para mostrar/ocultar
 * 6. Encadeamento de métodos (fluent interface)
 * 7. `adicionarPonto()` no gráfico de forma de onda
 *
 * ## Componentes no Nextion Editor
 *
 * | Tipo          | objname   | Pg | ID |
 * |---------------|-----------|----|----|
 * | Text          | t_temp    | 0  | 1  |
 * | Text          | t_hora    | 0  | 2  |
 * | Number        | n_val     | 0  | 3  |
 * | ProgressBar   | j_prog    | 0  | 4  |
 * | Slider        | sl_brilho | 0  | 5  |
 * | Button        | btn_cor   | 0  | 6  |
 * | Waveform      | s0        | 0  | 7  |
 * | Text          | t_log     | 0  | 8  |
 *
 * @author  professorThiago
 * @version 1.0.0
 */

#include <NextionUI.h>

// ---------------------------------------------------------------------------
// Objetos globais
// ---------------------------------------------------------------------------

NexDisplay       display;
NexTexto         tTemp    (0, 1, "t_temp");
NexTexto         tHora    (0, 2, "t_hora");
NexNumero        nVal     (0, 3, "n_val");
NexBarraProgresso jProg   (0, 4, "j_prog");
NexSlider        slBrilho (0, 5, "sl_brilho");
NexBotao         btnCor   (0, 6, "btn_cor");
NexGrafico       grafico  (0, 7, "s0");
NexTexto         tLog     (0, 8, "t_log");

// ---------------------------------------------------------------------------
// setup()
// ---------------------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    delay(500);

    display.begin(Serial2, 115200, 16, 17);
    display.brilho(80);
    display.escutar(slBrilho);
    display.escutar(btnCor);

    // ── Exemplo 1: texto simples ─────────────────────────────────────────
    tTemp.texto("22°C");

    // ── Exemplo 2: texto formatado (printf-style) ─────────────────────────
    float umidade = 65.4f;
    tTemp.setf("txt", "%.1f%% UR", umidade);

    // ── Exemplo 3: hora formatada ────────────────────────────────────────
    uint8_t h = 14, m = 32;
    tHora.setf("txt", "%02d:%02d", h, m);

    // ── Exemplo 4: número ─────────────────────────────────────────────────
    nVal.valor(42);

    // ── Exemplo 5: barra de progresso ────────────────────────────────────
    jProg.valor(75);

    // ── Exemplo 6: slider com limites ────────────────────────────────────
    slBrilho.minimo(0).maximo(100).valor(80);

    // Callback do slider — ajusta brilho do display ao arrastar
    slBrilho.aoSoltar([]() {
        uint32_t v;
        if (slBrilho.valor(v)) {
            display.brilho((uint8_t)v);
            tLog.setf("txt", "Brilho: %lu%%", v);
        }
    });

    // ── Exemplo 7: atributo genérico ─────────────────────────────────────
    // Muda qualquer atributo pelo nome — sem precisar de método específico
    tTemp.set("bco", RGB565(20, 80, 140));  // cor de fundo azul
    tTemp.set("pco", COR_BRANCO);            // texto branco
    tTemp.set("xcen", 1);                    // centralizado horizontalmente

    // ── Exemplo 8: encadeamento (fluent interface) ─────────────────────
    // Tudo em uma linha por componente
    btnCor.texto("Mudar Cor")
          .corFundo(RGB565(30, 30, 200))
          .corTexto(COR_BRANCO)
          .corFundoPressionado(RGB565(60, 60, 255));

    // Callback do botão de cor — alterna entre dois temas
    static bool temaEscuro = true;
    btnCor.aoSoltar([&]() {
        temaEscuro = !temaEscuro;
        if (temaEscuro) {
            tTemp.corFundo(RGB565(20, 30, 60)).corTexto(COR_BRANCO);
            tHora.corFundo(RGB565(20, 30, 60)).corTexto(COR_BRANCO);
            tLog.setf("txt", "Tema: Escuro");
        } else {
            tTemp.corFundo(COR_BRANCO).corTexto(COR_PRETO);
            tHora.corFundo(COR_BRANCO).corTexto(COR_PRETO);
            tLog.setf("txt", "Tema: Claro");
        }
    });

    // ── Exemplo 9: visibilidade ─────────────────────────────────────────
    tLog.visivel(true);

    tLog.texto("Pronto!");
}

// ---------------------------------------------------------------------------
// loop()
// ---------------------------------------------------------------------------

void loop() {
    display.atualizar();

    static uint32_t tUltimo = 0;
    if (millis() - tUltimo >= 500) {
        tUltimo = millis();

        // Simula leitura de sensor
        int temperatura = 20 + (millis() / 1000) % 15;
        tTemp.setf("txt", "%d°C", temperatura);

        // Atualiza barra de progresso com senoide (0-100)
        float seno = (sin(millis() / 2000.0f) + 1.0f) * 50.0f;
        jProg.valor((uint32_t)seno);

        // Adiciona ponto no gráfico (canal 0, valor 0-255)
        uint8_t ponto = (uint8_t)(seno * 2.55f);
        grafico.adicionarPonto(0, ponto);
    }

    // Atualiza relógio fictício
    static uint32_t tSeg = 0;
    static uint8_t seg = 0, min_ = 0, hor_ = 9;
    if (millis() - tSeg >= 1000) {
        tSeg = millis();
        if (++seg >= 60) { seg = 0; if (++min_ >= 60) { min_ = 0; hor_++; } }
        tHora.setf("txt", "%02d:%02d:%02d", hor_, min_, seg);
    }
}
