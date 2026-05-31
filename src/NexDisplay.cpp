/**
 * @file NexDisplay.cpp
 * @brief Implementação do NexDisplay.
 *
 * @author  professorThiago
 * @version 1.0.0
 * @license MIT
 */

#include "NexDisplay.h"
#include <stdarg.h>

static char _bufDisplay[NEX_BUFFER_CMD];

// ---------------------------------------------------------------------------
// Inicialização
// ---------------------------------------------------------------------------

bool NexDisplay::begin(HardwareSerial& serial, uint32_t baudrate,
                        int8_t pinoRX, int8_t pinoTX)
{
    memset(_lista, 0, sizeof(_lista));
    _totalEscuta = 0;
    _paginaAtual = 0;

    return nexIniciar(baudrate, pinoRX, pinoTX, &serial);
}

// ---------------------------------------------------------------------------
// Loop
// ---------------------------------------------------------------------------

void NexDisplay::atualizar()
{
    HardwareSerial* s = nexSerial();
    if (!s) return;
    while (s->available()) {
        _processarByte((uint8_t)s->read());
    }
}

void NexDisplay::_processarByte(uint8_t b)
{
    // Nextion termina todo evento com 0xFF 0xFF 0xFF
    if (_rxLen < sizeof(_rxBuf)) {
        _rxBuf[_rxLen++] = b;
    }

    // Detecta terminador
    if (_rxLen >= 3 &&
        _rxBuf[_rxLen-1] == 0xFF &&
        _rxBuf[_rxLen-2] == 0xFF &&
        _rxBuf[_rxLen-3] == 0xFF) {

        _processarEvento(_rxBuf, _rxLen - 3);
        _rxLen = 0;
    }
}

void NexDisplay::_processarEvento(const uint8_t* buf, uint8_t tam)
{
    if (tam < 1) return;

    switch (buf[0]) {

        // Evento de toque: 0x65 | pid | cid | evento(press=1/pop=0)
        case NEX_RET_TOQUE:
            if (tam >= 4) {
                _NEX_DBG("Toque pag=" + String(buf[1]) +
                         " comp=" + String(buf[2]) +
                         " evt=" + String(buf[3]));
                NexToque::despachar(_lista, buf[1], buf[2], (int32_t)buf[3]);
            }
            break;

        // Mudança de página: 0x66 | pagina
        case NEX_RET_MUDANCA_PAGINA:
            if (tam >= 2) {
                _paginaAtual = buf[1];
                _NEX_DBG("Pagina: " + String(_paginaAtual));
                if (_cbPagina) _cbPagina(_paginaAtual);
            }
            break;

        default:
            break;
    }
}

// ---------------------------------------------------------------------------
// Lista de escuta
// ---------------------------------------------------------------------------

bool NexDisplay::escutar(NexToque& componente)
{
    if (_totalEscuta >= NEX_MAX_COMPONENTES) {
        _NEX_DBG_ERR("escutar: lista cheia (" +
                     String(NEX_MAX_COMPONENTES) + ").");
        return false;
    }
    _lista[_totalEscuta++] = &componente;
    _lista[_totalEscuta]   = nullptr;
    return true;
}

void NexDisplay::limparEscuta()
{
    memset(_lista, 0, sizeof(_lista));
    _totalEscuta = 0;
}

// ---------------------------------------------------------------------------
// Navegação
// ---------------------------------------------------------------------------

void NexDisplay::irPara(const char* nome)
{
    nexEnviarCmdF("page %s", nome);
}

void NexDisplay::irPara(uint8_t indice)
{
    nexEnviarCmdF("page %u", indice);
}

// ---------------------------------------------------------------------------
// Comandos
// ---------------------------------------------------------------------------

void NexDisplay::cmdf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(_bufDisplay, sizeof(_bufDisplay), fmt, args);
    va_end(args);
    nexEnviarCmd(_bufDisplay);
}

void NexDisplay::brilho(uint8_t pct)
{
    if (pct > 100) pct = 100;
    nexEnviarCmdF("dim=%u", pct);
}

void NexDisplay::repouso(uint16_t timeoutS)
{
    nexEnviarCmdF("thsp=%u", timeoutS);
    nexEnviarCmdF("thup=%u", timeoutS > 0 ? 1 : 0);
}
