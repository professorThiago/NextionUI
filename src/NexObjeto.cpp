/**
 * @file NexObjeto.cpp
 * @brief Implementação da classe base NexObjeto.
 *
 * @author  professorThiago
 * @version 1.0.0
 * @license MIT
 */

#include "NexObjeto.h"
#include <stdarg.h>

static char _bufAttr[NEX_BUFFER_CMD];

// ---------------------------------------------------------------------------
// Construtor
// ---------------------------------------------------------------------------

NexObjeto::NexObjeto(uint8_t pid, uint8_t cid, const char* nome)
    : _pid(pid), _cid(cid), _nome(nome) {}

// ---------------------------------------------------------------------------
// set numérico
// ---------------------------------------------------------------------------

NexObjeto& NexObjeto::set(const char* attr, uint32_t valor)
{
    // Envia: <nome>.<attr>=<valor>
    nexEnviarCmdF("%s.%s=%lu", _nome, attr, valor);

    // Força redesenho
    nexEnviarCmdF("ref %s", _nome);

    if (!nexAguardarOk()) {
        _NEX_DBG_ERR("set num falhou: " + String(_nome) + "." + attr);
    }
    return *this;
}

// ---------------------------------------------------------------------------
// set texto
// ---------------------------------------------------------------------------

NexObjeto& NexObjeto::set(const char* attr, const char* texto)
{
    nexEnviarCmdF("%s.%s=\"%s\"", _nome, attr, texto);
    nexEnviarCmdF("ref %s", _nome);

    if (!nexAguardarOk()) {
        _NEX_DBG_ERR("set txt falhou: " + String(_nome) + "." + attr);
    }
    return *this;
}

// ---------------------------------------------------------------------------
// setf (texto formatado)
// ---------------------------------------------------------------------------

NexObjeto& NexObjeto::setf(const char* attr, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(_bufAttr, sizeof(_bufAttr), fmt, args);
    va_end(args);
    return set(attr, _bufAttr);
}

// ---------------------------------------------------------------------------
// get numérico
// ---------------------------------------------------------------------------

bool NexObjeto::get(const char* attr, uint32_t& destino)
{
    nexEnviarCmdF("get %s.%s", _nome, attr);
    return nexLerNumero(&destino);
}

// ---------------------------------------------------------------------------
// get texto
// ---------------------------------------------------------------------------

uint16_t NexObjeto::get(const char* attr, char* buffer, uint16_t tamanho)
{
    nexEnviarCmdF("get %s.%s", _nome, attr);
    return nexLerString(buffer, tamanho);
}

// ---------------------------------------------------------------------------
// Atalhos
// ---------------------------------------------------------------------------

NexObjeto& NexObjeto::corFundo(uint32_t cor)  { return set("bco", cor); }
NexObjeto& NexObjeto::corTexto(uint32_t cor)  { return set("pco", cor); }

NexObjeto& NexObjeto::visivel(bool mostrar)
{
    nexEnviarCmdF("vis %s,%d", _nome, mostrar ? 1 : 0);
    return *this;
}

NexObjeto& NexObjeto::atualizar()
{
    nexEnviarCmdF("ref %s", _nome);
    return *this;
}

NexObjeto& NexObjeto::cmd(const char* c)
{
    nexEnviarCmd(c);
    return *this;
}

// ---------------------------------------------------------------------------
// Debug
// ---------------------------------------------------------------------------

void NexObjeto::imprimirInfo() const
{
    _NEX_DBG_INFO("Obj: p=" + String(_pid) +
                  " id=" + String(_cid) +
                  " nome=" + String(_nome ? _nome : "(null)"));
}
