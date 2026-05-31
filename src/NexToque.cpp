/**
 * @file NexToque.cpp
 * @brief Implementação de NexToque.
 *
 * @author  professorThiago
 * @version 1.0.0
 * @license MIT
 */

#include "NexToque.h"

NexToque::NexToque(uint8_t pid, uint8_t cid, const char* nome)
    : NexObjeto(pid, cid, nome) {}

void NexToque::aoPressionar(CbToqueSimples cb) { _cbPressionarSimples = cb; }
void NexToque::aoSoltar    (CbToqueSimples cb) { _cbSoltarSimples     = cb; }

void NexToque::aoPressionar(CbToque cb, void* ptr)
{
    _cbPressionar  = cb;
    _ctxPressionar = ptr;
}

void NexToque::aoSoltar(CbToque cb, void* ptr)
{
    _cbSoltar   = cb;
    _ctxSoltar  = ptr;
}

void NexToque::removerCbPressionar()
{
    _cbPressionarSimples = nullptr;
    _cbPressionar        = nullptr;
    _ctxPressionar       = nullptr;
}

void NexToque::removerCbSoltar()
{
    _cbSoltarSimples = nullptr;
    _cbSoltar        = nullptr;
    _ctxSoltar       = nullptr;
}

void NexToque::_pressionar()
{
    if (_cbPressionarSimples) _cbPressionarSimples();
    else if (_cbPressionar)   _cbPressionar(_ctxPressionar);
}

void NexToque::_soltar()
{
    if (_cbSoltarSimples) _cbSoltarSimples();
    else if (_cbSoltar)   _cbSoltar(_ctxSoltar);
}

void NexToque::despachar(NexToque** lista, uint8_t pid,
                          uint8_t cid, int32_t evento)
{
    if (!lista) return;
    for (uint8_t i = 0; lista[i] != nullptr; i++) {
        NexToque* c = lista[i];
        if (c->_pid == pid && c->_cid == cid) {
            c->imprimirInfo();
            if (evento == NEX_EVENTO_PRESSIONAR) c->_pressionar();
            else if (evento == NEX_EVENTO_SOLTAR) c->_soltar();
            break;
        }
    }
}
