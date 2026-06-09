/**
 * @file NexAtualizador.cpp
 * @brief Implementação do protocolo de upload Nextion via serial.
 *
 * @author  professorThiago (https://github.com/professorThiago)
 * @version 1.0.0
 * @license MIT
 */

#include "NexAtualizador.h"

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

    // ── Passo 1: Enviar comando de upload na baud de operação ─────────────
    if (!_iniciarProtocolo(tamanho)) {
        return false;
    }

    // ── Passo 2: Trocar para baud de upload ───────────────────────────────
    if (_baudUpload != _baudOperacao) {
        debugVerbose("[NexOTA] Trocando baud: " + String(_baudOperacao) +
                     " -> " + String(_baudUpload));
        _serial.flush();
        delay(50);
        _serial.updateBaudRate(_baudUpload);
        delay(50);
    }

    // ── Passo 3: Enviar dados em blocos ───────────────────────────────────
    uint8_t bloco[NEX_UPLOAD_TAMANHO_BLOCO];
    uint32_t enviados = 0;
    uint32_t blocoNum = 0;
    uint8_t ultimoPct = 255;

    while (enviados < tamanho) {
        // Calcula tamanho deste bloco
        uint32_t restante = tamanho - enviados;
        size_t tamBloco = (restante < NEX_UPLOAD_TAMANHO_BLOCO)
                          ? restante
                          : NEX_UPLOAD_TAMANHO_BLOCO;

        // Lê da fonte
        size_t lido = fonte.readBytes(bloco, tamBloco);

        if (lido == 0) {
            _reportarErro(ErroNexUpload::LEITURA_FONTE,
                          "Falha ao ler dados da fonte");
            return false;
        }

        // Envia o bloco
        if (!_enviarBloco(bloco, lido)) {
            return false;
        }

        enviados += lido;
        blocoNum++;

        // Callback de progresso
        uint8_t pct = (uint8_t)((enviados * 100UL) / tamanho);
        if (pct != ultimoPct) {
            ultimoPct = pct;
            if (_cbProgresso) _cbProgresso(pct);
            debugVerbose("[NexOTA] Progresso: " + String(pct) + "% (" +
                         String(enviados) + "/" + String(tamanho) + ")");
        }
    }

    // ── Passo 4: Upload concluído — Nextion reinicia automaticamente ──────
    debugInfo("[NexOTA] Upload concluido! " + String(blocoNum) +
             " blocos enviados. Nextion reiniciando...");

    // Restaura baud de operação para quando o Nextion voltar
    if (_baudUpload != _baudOperacao) {
        _serial.flush();
        delay(100);
        _serial.updateBaudRate(_baudOperacao);
    }

    // Aguarda o Nextion reiniciar (~2-5 segundos dependendo do modelo)
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

    // ── Passo 1: Iniciar protocolo ────────────────────────────────────────
    if (!_iniciarProtocolo(tamanho)) {
        return false;
    }

    // ── Passo 2: Trocar baud ──────────────────────────────────────────────
    if (_baudUpload != _baudOperacao) {
        _serial.flush();
        delay(50);
        _serial.updateBaudRate(_baudUpload);
        delay(50);
    }

    // ── Passo 3: Enviar em blocos ─────────────────────────────────────────
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

    // ── Passo 4: Concluído ────────────────────────────────────────────────
    debugInfo("[NexOTA] Upload de buffer concluido!");

    if (_baudUpload != _baudOperacao) {
        _serial.flush();
        delay(100);
        _serial.updateBaudRate(_baudOperacao);
    }

    delay(3000);
    return true;
}

// =============================================================================
// _iniciarProtocolo — handshake com o Nextion
// =============================================================================

bool NexAtualizador::_iniciarProtocolo(uint32_t tamanho)
{
    // Garante que a serial está na baud de operação
    _serial.updateBaudRate(_baudOperacao);
    delay(100);

    // Limpa buffer serial
    _limparBuffer();

    // ── Tentativa 1: Conectar ao display ──────────────────────────────────
    // Envia string vazia para acordar o parser de comandos
    debugVerbose("[NexOTA] Conectando ao display...");
    _enviarComandoNex("");
    delay(100);
    _limparBuffer();

    // Envia o comando 'connect' para verificar se o display responde
    _enviarComandoNex("connect");
    delay(100);

    // Lê resposta (pode ser "comok..." ou lixo — o importante é que respondeu)
    bool conectado = _serial.available() > 0;
    _limparBuffer();

    if (!conectado) {
        // Tenta mais uma vez
        debugAviso("[NexOTA] Sem resposta — tentando novamente...");
        _enviarComandoNex("");
        delay(200);
        _enviarComandoNex("connect");
        delay(200);
        conectado = _serial.available() > 0;
        _limparBuffer();
    }

    if (!conectado) {
        _reportarErro(ErroNexUpload::TIMEOUT_CONEXAO,
                      "Display nao respondeu — verifique conexao e baud");
        return false;
    }

    debugVerbose("[NexOTA] Display conectado.");

    // ── Tentativa 2: Enviar comando de upload ─────────────────────────────
    // Formato: whmi-wri <tamanho>,<baud_upload>,0
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "whmi-wri %lu,%lu,0",
             (unsigned long)tamanho, (unsigned long)_baudUpload);

    debugVerbose("[NexOTA] Enviando: " + String(cmd));
    _enviarComandoNex(cmd);

    // Aguarda 0x05 (ACK) — o Nextion pode demorar a processar
    if (!_aguardarAck()) {
        _reportarErro(ErroNexUpload::SEM_RESPOSTA,
                      "Display nao aceitou comando de upload");
        return false;
    }

    debugVerbose("[NexOTA] Display pronto para receber dados.");
    return true;
}

// =============================================================================
// _enviarBloco — envia um bloco e aguarda ACK
// =============================================================================

bool NexAtualizador::_enviarBloco(const uint8_t* dados, size_t tamanho)
{
    _serial.write(dados, tamanho);
    _serial.flush(); // aguarda envio completo

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

            // Nextion pode enviar 0x08 (erro) ou outros bytes
            if (byte == 0x08) {
                debugErro("[NexOTA] Display retornou erro (0x08)");
                return false;
            }

            // Ignora outros bytes (possível lixo no buffer)
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
