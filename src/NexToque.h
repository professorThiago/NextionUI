/**
 * @file NexToque.h
 * @brief Componentes com suporte a eventos de toque.
 *
 * @details
 * `NexToque` estende `NexObjeto` adicionando a capacidade de registrar
 * callbacks para eventos de pressionar e soltar.
 *
 * ### Modos de uso
 *
 * **Callback simples (lambda — recomendado):**
 * @code
 * NexBotao btn(0, 2, "b0");
 * btn.aoPressionar([]() { Serial.println("Pressionado!"); });
 * btn.aoSoltar([]()     { Serial.println("Solto!"); });
 * @endcode
 *
 * **Callback com ponteiro de contexto (compatibilidade):**
 * @code
 * void minhaFuncao(void* ptr) {
 *     NexBotao* b = (NexBotao*)ptr;
 *     // ...
 * }
 * btn.aoPressionar(minhaFuncao, &btn);
 * @endcode
 *
 * ### Integração com NexDisplay
 *
 * Para que os callbacks sejam chamados automaticamente no `loop()`,
 * registre os componentes no `NexDisplay`:
 *
 * @code
 * NexDisplay display;
 * NexBotao btn(0, 2, "b0");
 *
 * void setup() {
 *     display.begin(Serial2, 115200, 16, 17);
 *     display.escutar(btn);
 *     btn.aoSoltar([]() { Serial.println("Clicou!"); });
 * }
 *
 * void loop() {
 *     display.atualizar();  // processa eventos de toque
 * }
 * @endcode
 *
 * @author  professorThiago
 * @version 1.0.0
 * @license MIT
 */

#ifndef __NEXTOQUE_H__
#define __NEXTOQUE_H__

#include "NexObjeto.h"

/** @brief Tipo de callback com ponteiro de contexto (estilo C). */
typedef void (*CbToque)(void* ptr);

/** @brief Tipo de callback simples sem parâmetros (lambdas). */
typedef void (*CbToqueSimples)(void);

/** @brief Evento de toque: pressionar (dedo/caneta tocando a tela). */
#define NEX_EVENTO_PRESSIONAR  0x01

/** @brief Evento de toque: soltar (dedo/caneta saindo da tela). */
#define NEX_EVENTO_SOLTAR      0x00

/**
 * @brief Classe base para componentes com eventos de toque.
 *
 * @details
 * Herda de `NexObjeto` e adiciona suporte a callbacks para
 * pressionar (`push`) e soltar (`pop`).
 */
class NexToque : public NexObjeto {
public:
    /**
     * @copydoc NexObjeto::NexObjeto
     */
    NexToque(uint8_t pid, uint8_t cid, const char* nome);

    // -----------------------------------------------------------------------
    // Registro de callbacks
    // -----------------------------------------------------------------------

    /**
     * @brief Registra callback chamado ao pressionar (lambda).
     *
     * @param cb  Função `void cb()`.
     *
     * @par Exemplo
     * @code
     * btn.aoPressionar([]() { Serial.println("Pressionado!"); });
     * @endcode
     */
    void aoPressionar(CbToqueSimples cb);

    /**
     * @brief Registra callback chamado ao soltar (lambda).
     *
     * @param cb  Função `void cb()`.
     *
     * @par Exemplo
     * @code
     * btn.aoSoltar([]() { ligarLampada(); });
     * @endcode
     */
    void aoSoltar(CbToqueSimples cb);

    /**
     * @brief Registra callback ao pressionar com ponteiro de contexto.
     *
     * @param cb   Função `void cb(void* ptr)`.
     * @param ptr  Ponteiro passado para o callback.
     */
    void aoPressionar(CbToque cb, void* ptr = nullptr);

    /**
     * @brief Registra callback ao soltar com ponteiro de contexto.
     *
     * @param cb   Função `void cb(void* ptr)`.
     * @param ptr  Ponteiro passado para o callback.
     */
    void aoSoltar(CbToque cb, void* ptr = nullptr);

    /**
     * @brief Remove o callback de pressionar.
     */
    void removerCbPressionar();

    /**
     * @brief Remove o callback de soltar.
     */
    void removerCbSoltar();

    // -----------------------------------------------------------------------
    // Iteração interna (chamada pelo NexDisplay)
    // -----------------------------------------------------------------------

    /**
     * @brief Percorre a lista e dispara o callback do componente correto.
     *
     * @details
     * Chamado internamente pelo `NexDisplay::atualizar()`. Você não
     * precisa chamar este método diretamente.
     *
     * @param lista   Array de ponteiros para NexToque, terminado em `NULL`.
     * @param pid     ID da página do evento recebido.
     * @param cid     ID do componente do evento recebido.
     * @param evento  NEX_EVENTO_PRESSIONAR ou NEX_EVENTO_SOLTAR.
     */
    static void despachar(NexToque** lista, uint8_t pid,
                           uint8_t cid, int32_t evento);

private:
    void _pressionar();
    void _soltar();

    CbToqueSimples _cbPressionarSimples = nullptr;
    CbToqueSimples _cbSoltarSimples     = nullptr;
    CbToque        _cbPressionar        = nullptr;
    void*          _ctxPressionar       = nullptr;
    CbToque        _cbSoltar            = nullptr;
    void*          _ctxSoltar          = nullptr;
};

#endif /* __NEXTOQUE_H__ */
