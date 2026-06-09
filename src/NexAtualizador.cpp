/**
 * @file NexAtualizador.cpp
 * @brief Implementação do protocolo de upload Nextion via serial.
 *
 * @author  professorThiago (https://github.com/professorThiago)
 * @version 1.1.0
 * @license MIT
 */

#include "NexAtualizador.h"

// ── Bauds para auto-detecção (mesma ordem do Nextion Editor) ──────────────
static const uint32_t _BAUDS_TESTE[] = {
    9600, 115200, 19200, 38400, 57600, 230400, 4800, 512000, 921600, 2400
};
static const uint8_t _NUM_BAUDS = sizeof(_BAUDS_TESTE) / sizeof(_BAUDS_TESTE[0]);

// =============================================================================
// Construtor
// =============================================================================

NexAtualizador::NexAtualizador(HardwareSerial& serial)
    : _serial(serial)
{
}

// =============================================================================
// atualizar — Stream (HTTP, SD, SPIFFS)
// =============================================================================

bool NexAtualizador::atualizar(Stream& fonte, uint32_t tamanho)
{
    if (tamanho == 0) {
        _reportarErro(ErroNexUpload::TAMANHO_ZERO,
                      "Tamanho do arquivo .tft e zero");
        return false;
    }

    debugInfo("[NexOTA] Iniciando upload — " + String(tamanho) + " bytes");

    // ── Passo 1: Auto-detectar baud e enviar comando de upload ────────
    if (!_iniciarProtocolo(tamanho)) {
        return false;
    }

    // ── Passo 2: Trocar para baud de upload ───────────────────────────
 //   if (_baudUpload != _baudDetectada) {
 //       debugVerbose("[NexOTA] Trocando baud: " + String(_baudDetectada) +
 //                    " -> " + String(_baudUpload));
 //       _serial.flush();
 //       delay(500);
 //       _serial.updateBaudRate(_baudUpload);
 //       delay(500);
    }

    // ── Passo 3: Enviar dados em blocos ───────────────────────────────
    uint8_t bloco[NEX_UPLOAD_TAMANHO_BLOCO];
    uint32_t enviados = 0;
    uint32_t blocoNum = 0;
    uint8_t ultimoPct = 255;

    while (enviados < tamanho) {
        uint32_t restante = tamanho - enviados;
        size_t tamBloco = (restante < NEX_UPLOAD_TAMANHO_BLOCO)
                          ? restante
                          : NEX_UPLOAD_TAMANHO_BLOCO;

        size_t lido = fonte.readBytes(bloco, tamBloco);

        if (lido == 0) {
            _reportarErro(ErroNexUpload::LEITURA_FONTE,
                          "Falha ao ler dados da fonte");
            return false;
        }

        if (!_enviarBloco(bloco, lido)) {
            return false;
        }

        enviados += lido;
        blocoNum++;

        uint8_t pct = (uint8_t)((enviados * 100UL) / tamanho);
        if (pct != ultimoPct) {
            ultimoPct = pct;
            if (_cbProgresso) _cbProgresso(pct);
            debugVerbose("[NexOTA] Progresso: " + String(pct) + "% (" +
                         String(enviados) + "/" + String(tamanho) + ")");
        }
    }

    // ── Passo 4: Concluído — Nextion reinicia automaticamente ─────────
    debugInfo("[NexOTA] Upload concluido! " + String(blocoNum) +
             " blocos enviados. Nextion reiniciando...");

    // Restaura baud de operação para quando o Nextion voltar
    _serial.flush();
    delay(500);
    _serial.updateBaudRate(_baudDetectada);

    delay(3000);
    return true;
}

// =============================================================================
// atualizar — buffer na memória
// =============================================================================

bool NexAtualizador::atualizar(const uint8_t* dados, uint32_t tamanho)
{
    if (!dados || tamanho == 0) {
        _reportarErro(ErroNexUpload::TAMANHO_ZERO,
                      "Dados nulos ou tamanho zero");
        return false;
    }

    debugInfo("[NexOTA] Iniciando upload de buffer — " +
             String(tamanho) + " bytes");

    if (!_iniciarProtocolo(tamanho)) {
        return false;
    }

//    if (_baudUpload != _baudDetectada) {
//        _serial.flush();
//        delay(500);
//        _serial.updateBaudRate(_baudUpload);
//        delay(500);
//    }

    uint32_t enviados = 0;
    uint8_t ultimoPct = 255;

    while (enviados < tamanho) {
        uint32_t restante = tamanho - enviados;
        size_t tamBloco = (restante < NEX_UPLOAD_TAMANHO_BLOCO)
                          ? restante
                          : NEX_UPLOAD_TAMANHO_BLOCO;

        if (!_enviarBloco(dados + enviados, tamBloco)) {
            return false;
        }

        enviados += tamBloco;

        uint8_t pct = (uint8_t)((enviados * 100UL) / tamanho);
        if (pct != ultimoPct) {
            ultimoPct = pct;
            if (_cbProgresso) _cbProgresso(pct);
        }
    }

    debugInfo("[NexOTA] Upload de buffer concluido!");

    _serial.flush();
    delay(500);
    _serial.updateBaudRate(_baudDetectada);

    delay(3000);
    return true;
}

// =============================================================================
// _detectarBaud — testa várias velocidades até o display responder
// =============================================================================

bool NexAtualizador::_detectarBaud()
{
    debugInfo("[NexOTA] Auto-detectando baud rate do display...");

    for (uint8_t i = 0; i < _NUM_BAUDS; i++) {
        uint32_t baud = _BAUDS_TESTE[i];

        _serial.updateBaudRate(baud);
        delay(100);
        _limparBuffer();

        // Envia string vazia para acordar o parser
        _enviarComandoNex("");
        delay(50);
        _limparBuffer();

        // Envia connect e verifica resposta
        _enviarComandoNex("connect");
        delay(150);

        if (_serial.available() > 0) {
            // Lê a resposta para confirmar que é válida
            String resp = "";
            while (_serial.available()) {
                char c = _serial.read();
                if (c >= 0x20 && c < 0x7F) resp += c;
            }

            debugInfo("[NexOTA] Display encontrado em " + String(baud) +
                      " baud (resposta: " + resp + ")");
            _baudDetectada = baud;
            return true;
        }

        debugVerbose("[NexOTA] Sem resposta em " + String(baud));
    }

    debugErro("[NexOTA] Display nao encontrado em nenhuma velocidade");
    return false;
}

// =============================================================================
// _iniciarProtocolo — auto-detecção + handshake com o Nextion
// =============================================================================

bool NexAtualizador::_iniciarProtocolo(uint32_t tamanho)
{
    // ── Passo 1: Auto-detectar baud ───────────────────────────────────
    if (!_detectarBaud()) {
        _reportarErro(ErroNexUpload::TIMEOUT_CONEXAO,
                      "Display nao encontrado — verifique conexao");
        return false;
    }

    _serial.updateBaudRate(_baudDetectada);
    delay(100);
    _limparBuffer();

    // ── Passo 2: Mudar baud do display (como o Nextion Editor) ────────
    if (_baudUpload != _baudDetectada) {
        char cmdBaud[32];
        snprintf(cmdBaud, sizeof(cmdBaud), "baud=%lu",
                 (unsigned long)_baudUpload);

        debugInfo("[NexOTA] Mudando baud do display: " +
                  String(_baudDetectada) + " -> " + String(_baudUpload));

        _enviarComandoNex(cmdBaud);
        delay(100);

        // ESP32 troca para acompanhar
        _serial.flush();
        delay(50);
        _serial.updateBaudRate(_baudUpload);
        delay(100);
        _limparBuffer();

        // Confirma que o display responde na baud nova
        _enviarComandoNex("");
        delay(50);
        _enviarComandoNex("connect");
        delay(150);

        if (_serial.available() == 0) {
            debugAviso("[NexOTA] Display nao respondeu em " +
                       String(_baudUpload) + " — voltando para " +
                       String(_baudDetectada));
            _serial.updateBaudRate(_baudDetectada);
            delay(100);
            _baudUpload = _baudDetectada; // fallback
        } else {
            debugInfo("[NexOTA] Display confirmado em " +
                      String(_baudUpload) + " baud");
            _limparBuffer();
        }
    }

    // ── Passo 3: Enviar comando de upload na baud atual ───────────────
    _enviarComandoNex("");
    delay(100);
    _limparBuffer();

    char cmd[64];
    snprintf(cmd, sizeof(cmd), "whmi-wri %lu,%lu,0",
             (unsigned long)tamanho, (unsigned long)_baudUpload);

    debugInfo("[NexOTA] Enviando: " + String(cmd));
    _enviarComandoNex(cmd);

    if (!_aguardarAck()) {
        debugAviso("[NexOTA] Sem ACK — tentando novamente...");
        delay(500);
        _limparBuffer();
        _enviarComandoNex(cmd);

        if (!_aguardarAck()) {
            _reportarErro(ErroNexUpload::SEM_RESPOSTA,
                          "Display nao aceitou comando de upload");
            return false;
        }
    }

    debugInfo("[NexOTA] Display pronto para receber dados.");
    return true;
}

// =============================================================================
// _enviarBloco — envia um bloco e aguarda ACK
// =============================================================================

bool NexAtualizador::_enviarBloco(const uint8_t* dados, size_t tamanho)
{
    _serial.write(dados, tamanho);
    _serial.flush();

    if (!_aguardarAck()) {
        _reportarErro(ErroNexUpload::BLOCO_SEM_ACK,
                      "Bloco enviado sem confirmacao do display");
        return false;
    }

    return true;
}

// =============================================================================
// _aguardarAck — espera byte 0x05 do Nextion
// =============================================================================

bool NexAtualizador::_aguardarAck()
{
    uint32_t inicio = millis();

    while (millis() - inicio < _tempoLimite) {
        if (_serial.available()) {
            uint8_t byte = _serial.read();

            if (byte == NEX_UPLOAD_ACK) {
                return true;
            }

            if (byte == 0x08) {
                debugErro("[NexOTA] Display retornou erro (0x08)");
                return false;
            }

            debugTudo("[NexOTA] Byte inesperado: 0x" +
                      String(byte, HEX));
        }

        delay(1);
    }

    debugErro("[NexOTA] Timeout aguardando ACK (" +
             String(_tempoLimite) + "ms)");
    return false;
}

// =============================================================================
// _limparBuffer — descarta dados pendentes na serial
// =============================================================================

void NexAtualizador::_limparBuffer()
{
    while (_serial.available()) {
        _serial.read();
    }
}

// =============================================================================
// _enviarComandoNex — envia comando com terminadores 0xFF 0xFF 0xFF
// =============================================================================

void NexAtualizador::_enviarComandoNex(const char* cmd)
{
    _serial.print(cmd);
    _serial.write(0xFF);
    _serial.write(0xFF);
    _serial.write(0xFF);
}

// =============================================================================
// _reportarErro
// =============================================================================

void NexAtualizador::_reportarErro(ErroNexUpload codigo,
                                    const char* mensagem)
{
    debugErro("[NexOTA] Erro " + String((int)codigo) + ": " +
             String(mensagem));
    if (_cbErro) _cbErro(codigo, mensagem);
}
