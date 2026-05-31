/**
 * @file 01_OlaMundo.ino
 * @brief Exemplo introdutório — inicializar o display e atualizar um texto.
 *
 * ## O que este exemplo faz
 *
 * 1. Inicializa a comunicação com o Nextion (Serial2, pinos 16/17, 115200 baud).
 * 2. Navega para a página 0 e exibe "Olá, mundo!" no componente `t0`.
 * 3. Atualiza um contador a cada segundo no componente `n0`.
 *
 * ## Display Nextion necessário
 *
 * Crie um projeto no Nextion Editor com:
 * - Página 0 (`p0`)
 * - Componente Text, objname = `t0`
 * - Componente Number, objname = `n0`
 *
 * ## Conexão de hardware
 *
 * | Nextion | ESP32      |
 * |---------|------------|
 * | TX      | GPIO 16    |
 * | RX      | GPIO 17    |
 * | VCC     | 5V         |
 * | GND     | GND        |
 *
 * @note O GPIO 16 do ESP32 é o pino RX (recebe dados do TX do Nextion).
 *       O GPIO 17 do ESP32 é o pino TX (envia dados para o RX do Nextion).
 *
 * @author  professorThiago
 * @version 1.0.0
 */

#include <NextionUI.h>

// ---------------------------------------------------------------------------
// Objetos globais
// ---------------------------------------------------------------------------

NexDisplay display;

// Componentes: (pagina, id, objname)
NexTexto  tMensagem(0, 1, "t0");
NexNumero nContador(0, 2, "n0");

// ---------------------------------------------------------------------------
// setup()
// ---------------------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("Iniciando...");

    // Inicializa o display
    // Serial2, 115200 baud, RX=GPIO16, TX=GPIO17
    if (!display.begin(Serial2, 115200, 16, 17)) {
        Serial.println("ERRO: falha ao inicializar Nextion!");
        while (true) delay(1000);
    }

    // Define brilho inicial (80%)
    display.brilho(80);

    // Navega para a página 0
    display.irPara(0);

    // Atualiza o texto de boas-vindas
    tMensagem.texto("Ola, mundo!");

    // Zera o contador
    nContador.valor(0);

    Serial.println("Display pronto.");
}

// ---------------------------------------------------------------------------
// loop()
// ---------------------------------------------------------------------------

uint32_t contador = 0;

void loop() {
    // Processa eventos (toque, mudança de página, etc.)
    display.atualizar();

    // Atualiza o contador a cada 1 segundo
    static uint32_t tUltimo = 0;
    if (millis() - tUltimo >= 1000) {
        tUltimo = millis();
        nContador.valor(++contador);
        Serial.println("Contador: " + String(contador));
    }
}
