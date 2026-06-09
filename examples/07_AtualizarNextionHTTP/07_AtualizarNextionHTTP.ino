/**
 * @file AtualizarNextionHTTP.ino
 * @brief Atualiza a tela Nextion baixando o .tft de um servidor HTTP.
 *
 * ## O que este exemplo faz
 *
 * 1. Conecta ao WiFi.
 * 2. Baixa o arquivo .tft de uma URL.
 * 3. Envia via serial para o Nextion usando o protocolo de upload.
 * 4. Nextion reinicia com a nova interface.
 *
 * ## Conexões
 *
 * | ESP32    | Nextion        |
 * |----------|----------------|
 * | TX2 (17) | RX (fio azul)  |
 * | RX2 (16) | TX (fio amarelo) |
 * | 5V       | 5V (vermelho)  |
 * | GND      | GND (preto)    |
 *
 * @author  professorThiago
 * @version 1.0.0
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <NexAtualizador.h>

// ── Configurações ─────────────────────────────────────────────────────────

const char* SSID  = "MinhaRede";
const char* SENHA = "MinhaSenha";

// URL do arquivo .tft no servidor
const char* URL_TFT = "https://servidor.com/api/firmware/download/nextion/1";

// Baud rate que o Nextion está usando atualmente (configurado no Nextion Editor)
#define NEXTION_BAUD_OPERACAO  921600

// Pinos Serial2 para o Nextion
#define NEXTION_RX  16
#define NEXTION_TX  17

// ── Instância ─────────────────────────────────────────────────────────────

NexAtualizador nexOta(Serial2);

// ── setup() ───────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Atualizador Nextion via HTTP ===\n");

    // Inicia Serial2 para o Nextion
    Serial2.begin(NEXTION_BAUD_OPERACAO, SERIAL_8N1, NEXTION_RX, NEXTION_TX);

    // Conecta ao WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, SENHA);
    Serial.print("WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print(".");
    }
    Serial.println(" OK — IP: " + WiFi.localIP().toString());

    // ── Configuração do NexAtualizador ────────────────────────────────────

    nexOta.definirBaudOperacao(NEXTION_BAUD_OPERACAO);
    nexOta.definirBaudUpload(115200);  // 115200 é mais seguro para upload

    nexOta.aoProgresso([](uint8_t pct) {
        Serial.printf("\rUpload Nextion: %3d%% ", pct);
        for (uint8_t i = 0; i < 20; i++) {
            Serial.print(i < pct / 5 ? "#" : ".");
        }
    });

    nexOta.aoErro([](ErroNexUpload codigo, const char* msg) {
        Serial.printf("\nErro [%d]: %s\n", (int)codigo, msg);
    });

    // ── Download e upload ─────────────────────────────────────────────────

    Serial.println("Baixando .tft de: " + String(URL_TFT));

    HTTPClient http;
    http.begin(URL_TFT);
    http.setTimeout(30000); // 30s timeout para download

    int codigo = http.GET();

    if (codigo != 200) {
        Serial.printf("Erro HTTP: %d\n", codigo);
        http.end();
        return;
    }

    uint32_t tamanho = http.getSize();
    Serial.printf("Arquivo: %u bytes\n", tamanho);

    if (tamanho == 0) {
        Serial.println("Erro: tamanho desconhecido (servidor deve enviar Content-Length)");
        http.end();
        return;
    }

    // Pega o stream HTTP e passa direto para o Nextion
    WiFiClient* stream = http.getStreamPtr();

    Serial.println("Iniciando upload para o Nextion...\n");

    bool sucesso = nexOta.atualizar(*stream, tamanho);

    http.end();

    // ── Resultado ─────────────────────────────────────────────────────────

    Serial.println();
    if (sucesso) {
        Serial.println("Tela atualizada com sucesso!");
        Serial.println("O Nextion reiniciou com a nova interface.");
    } else {
        Serial.println("Falha na atualizacao da tela.");
    }
}

// ── loop() ────────────────────────────────────────────────────────────────

void loop() {
    // Nada — upload feito no setup
    delay(1000);
}
