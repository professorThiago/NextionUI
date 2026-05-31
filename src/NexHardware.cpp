/**
 * @file NexHardware.cpp
 * @brief Implementação da camada de hardware NextionUI.
 *
 * @author  professorThiago
 * @version 1.0.0
 * @license MIT
 */

#include "NexHardware.h"
#include <stdarg.h>

// ---------------------------------------------------------------------------
// Estado interno
// ---------------------------------------------------------------------------

static HardwareSerial* _serial     = &Serial1;
static int8_t          _pinoRX     = -1;
static int8_t          _pinoTX     = -1;
static uint32_t        _baudrate   = 9600;

// Buffer de comando fixo — zero alocações no heap
static char _bufCmd[NEX_BUFFER_CMD];

// ---------------------------------------------------------------------------
// Inicialização
// ---------------------------------------------------------------------------

bool nexIniciar(uint32_t baudrate, int8_t pinoRX, int8_t pinoTX,
                HardwareSerial* serial)
{
    if (!serial) {
        _NEX_DBG_ERR("nexIniciar: serial nulo.");
        return false;
    }

    _serial   = serial;
    _baudrate = baudrate;
    _pinoRX   = pinoRX;
    _pinoTX   = pinoTX;

#if defined(ESP32)
    if (pinoRX >= 0 && pinoTX >= 0) {
        _serial->begin(baudrate, SERIAL_8N1, pinoRX, pinoTX);
    } else {
        _serial->begin(baudrate);
    }
#else
    _serial->begin(baudrate);
#endif

    delay(300);

    // Descarta bytes residuais do boot do Nextion
    while (_serial->available()) _serial->read();

    // Suprime todos os retornos de comando (mais rápido, menos ruído)
    nexEnviarCmd("");
    nexEnviarCmd("bkcmd=0");
    delay(50);
    while (_serial->available()) _serial->read();

    _NEX_DBG_INFO("Nextion inicializado.");
    return true;
}

void nexEncerrar()
{
    if (_serial) _serial->end();
}

HardwareSerial* nexSerial()
{
    return _serial;
}

// ---------------------------------------------------------------------------
// Envio de comandos
// ---------------------------------------------------------------------------

void nexEnviarCmd(const char* cmd)
{
    if (!_serial) return;

    // Descarta buffer de entrada antes de enviar
    while (_serial->available()) _serial->read();

    _serial->print(cmd);
    _serial->write(0xFF);
    _serial->write(0xFF);
    _serial->write(0xFF);
}

void nexEnviarCmdF(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(_bufCmd, sizeof(_bufCmd), fmt, args);
    va_end(args);
    nexEnviarCmd(_bufCmd);
}

// ---------------------------------------------------------------------------
// Recepção
// ---------------------------------------------------------------------------

bool nexAguardarOk(uint32_t timeout)
{
#if NEX_AGUARDAR_CONFIRMACAO == 0
    return true;
#else
    uint8_t buf[4] = {0};
    _serial->setTimeout(timeout);
    if (4 != _serial->readBytes((char*)buf, 4)) {
        _NEX_DBG_ERR("nexAguardarOk: timeout.");
        return false;
    }
    bool ok = (buf[0] == NEX_RET_CMD_OK &&
               buf[1] == 0xFF && buf[2] == 0xFF && buf[3] == 0xFF);
    if (!ok) {
        _NEX_DBG_ERR("nexAguardarOk: resposta inesperada.");
    }
    return ok;
#endif
}

bool nexLerNumero(uint32_t* numero, uint32_t timeout)
{
    if (!numero) return false;

    uint8_t buf[8] = {0};
    _serial->setTimeout(timeout);

    if (8 != _serial->readBytes((char*)buf, 8)) {
        _NEX_DBG_ERR("nexLerNumero: timeout.");
        return false;
    }

    if (buf[0] != NEX_RET_NUMERO ||
        buf[5] != 0xFF || buf[6] != 0xFF || buf[7] != 0xFF) {
        _NEX_DBG_ERR("nexLerNumero: formato invalido.");
        return false;
    }

    // Little-endian: bytes 1..4
    *numero = ((uint32_t)buf[4] << 24) |
              ((uint32_t)buf[3] << 16) |
              ((uint32_t)buf[2] <<  8) |
               (uint32_t)buf[1];

    _NEX_DBG("nexLerNumero: " + String(*numero));
    return true;
}

uint16_t nexLerString(char* buffer, uint16_t tamanho, uint32_t timeout)
{
    if (!buffer || tamanho == 0) return 0;

    bool   iniciou   = false;
    uint8_t ff_count = 0;
    uint16_t idx     = 0;
    uint8_t  c       = 0;
    uint32_t inicio  = millis();

    while (millis() - inicio <= timeout) {
        while (_serial->available()) {
            c = _serial->read();

            if (!iniciou) {
                if (c == NEX_RET_STRING) iniciou = true;
                continue;
            }

            if (c == 0xFF) {
                ff_count++;
                if (ff_count >= 3) goto fim;
            } else {
                ff_count = 0;
                if (idx + 1 < tamanho) {
                    buffer[idx++] = (char)c;
                }
            }
        }
        if (ff_count >= 3) break;
    }

fim:
    buffer[idx] = '\0';
    _NEX_DBG("nexLerString: " + String(buffer));
    return idx;
}
