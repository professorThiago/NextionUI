/**
 * @file NexObjeto.h
 * @brief Classe base de todos os componentes Nextion.
 *
 * @details
 * `NexObjeto` é a raiz da hierarquia de componentes. Ela encapsula
 * o endereço do componente no display (página + ID + nome) e fornece
 * os métodos genéricos que eliminam a duplicação de código:
 *
 * ### Métodos genéricos — a grande melhoria
 *
 * Em vez de implementar `Get_background_color_bco()`,
 * `Set_background_color_bco()`, etc. em cada componente separadamente,
 * esta classe oferece dois métodos genéricos:
 *
 * @code
 * // Setar qualquer atributo numérico
 * btn.set("bco", 1024);    // cor de fundo
 * btn.set("pco", 65535);   // cor do texto
 * btn.set("val", 42);      // valor
 *
 * // Setar atributo de texto
 * btn.set("txt", "Olá");
 *
 * // Ler qualquer atributo numérico
 * uint32_t cor;
 * btn.get("bco", cor);
 *
 * // Ler atributo de texto
 * char buf[32];
 * btn.get("txt", buf, sizeof(buf));
 * @endcode
 *
 * ### Fluent interface (encadeamento)
 *
 * Os métodos retornam `NexObjeto&` permitindo encadeamento:
 *
 * @code
 * btn.corFundo(1024)
 *    .corTexto(65535)
 *    .visivel(true)
 *    .atualizar();
 * @endcode
 *
 * @author  professorThiago
 * @version 1.0.0
 * @license MIT
 */

#ifndef __NEXOBJETO_H__
#define __NEXOBJETO_H__

#include <Arduino.h>
#include "NexConfig.h"
#include "NexHardware.h"

/**
 * @brief Classe base de todos os componentes Nextion.
 *
 * @details
 * Todo componente Nextion é identificado por três valores:
 * - **pid** (Page ID): índice da página onde o componente está (0-based).
 * - **cid** (Component ID): identificador único dentro da página.
 * - **nome**: string igual ao campo "objname" no Nextion Editor.
 *
 * @note O `nome` deve ser uma string literal ou ponteiro válido durante
 * toda a vida do objeto — ela não é copiada internamente.
 */
class NexObjeto {
public:
    // -----------------------------------------------------------------------
    // Construtor
    // -----------------------------------------------------------------------

    /**
     * @brief Cria um componente Nextion.
     *
     * @param pid   ID da página (0 = primeira página).
     * @param cid   ID do componente dentro da página.
     * @param nome  Nome do componente no Nextion Editor (objname).
     *
     * @par Exemplo
     * @code
     * // Botão "b0" na página 0, ID 2
     * NexBotao btn(0, 2, "b0");
     *
     * // Texto "t_temp" na página 1, ID 5
     * NexTexto tTemp(1, 5, "t_temp");
     * @endcode
     */
    NexObjeto(uint8_t pid, uint8_t cid, const char* nome);

    // -----------------------------------------------------------------------
    // Métodos genéricos de atributo — o núcleo da API
    // -----------------------------------------------------------------------

    /**
     * @brief Define um atributo numérico do componente.
     *
     * @details
     * Envia o comando `<nome>.<attr>=<valor>` seguido de `ref <nome>`
     * para atualizar o display. Usa buffer fixo — sem alocação de heap.
     *
     * @param attr   Nome do atributo Nextion (ex: "bco", "pco", "val").
     * @param valor  Valor numérico a definir.
     *
     * @return Referência a este objeto (permite encadeamento).
     *
     * @par Atributos numéricos comuns
     * | Atributo | Significado                   |
     * |----------|-------------------------------|
     * | `val`    | Valor numérico do componente  |
     * | `bco`    | Cor de fundo (RGB565)         |
     * | `pco`    | Cor do texto (RGB565)         |
     * | `font`   | Índice da fonte               |
     * | `xcen`   | Alinhamento horizontal (0-2)  |
     * | `ycen`   | Alinhamento vertical (0-2)    |
     * | `en`     | Habilitado (1) / desabilitado (0) |
     *
     * @par Exemplo
     * @code
     * txt.set("bco", 1024);     // muda cor de fundo
     * txt.set("pco", 65535);    // muda cor do texto
     * num.set("val", 42);       // muda valor
     * timer.set("en", 1);       // habilita timer
     * @endcode
     */
    NexObjeto& set(const char* attr, uint32_t valor);

    /**
     * @brief Define um atributo de texto do componente.
     *
     * @param attr   Nome do atributo (geralmente "txt").
     * @param texto  String a definir (terminada em `\0`).
     *
     * @return Referência a este objeto (permite encadeamento).
     *
     * @par Exemplo
     * @code
     * btn.set("txt", "LIGAR");
     * txt.set("txt", "22°C");
     * @endcode
     */
    NexObjeto& set(const char* attr, const char* texto);

    /**
     * @brief Define um atributo de texto com formatação printf.
     *
     * @details
     * Usa buffer interno fixo — sem `String`, sem alocação dinâmica.
     *
     * @param attr  Nome do atributo.
     * @param fmt   String de formato (estilo printf).
     * @param ...   Argumentos de formatação.
     *
     * @return Referência a este objeto (permite encadeamento).
     *
     * @par Exemplo
     * @code
     * txt.setf("txt", "%d°C", temperatura);
     * txt.setf("txt", "%.1f%%", umidade);
     * txt.setf("txt", "%02d:%02d", hora, minuto);
     * @endcode
     */
    NexObjeto& setf(const char* attr, const char* fmt, ...);

    /**
     * @brief Lê um atributo numérico do componente.
     *
     * @details
     * Envia `get <nome>.<attr>` e aguarda a resposta.
     *
     * @param      attr    Nome do atributo.
     * @param[out] destino Variável que receberá o valor lido.
     *
     * @return true  se lido com sucesso.
     * @return false se timeout ou erro de comunicação.
     *
     * @par Exemplo
     * @code
     * uint32_t posicao;
     * if (slider.get("val", posicao)) {
     *     Serial.println("Slider em: " + String(posicao));
     * }
     * @endcode
     */
    bool get(const char* attr, uint32_t& destino);

    /**
     * @brief Lê um atributo de texto do componente.
     *
     * @param      attr     Nome do atributo.
     * @param[out] buffer   Buffer que receberá o texto.
     * @param      tamanho  Tamanho do buffer (incluindo `\0`).
     *
     * @return Comprimento do texto lido.
     *
     * @par Exemplo
     * @code
     * char texto[32];
     * txt.get("txt", texto, sizeof(texto));
     * Serial.println(texto);
     * @endcode
     */
    uint16_t get(const char* attr, char* buffer, uint16_t tamanho);

    // -----------------------------------------------------------------------
    // Atalhos para atributos comuns (fluent interface)
    // -----------------------------------------------------------------------

    /**
     * @brief Define a cor de fundo (atributo `bco`).
     *
     * @param cor  Cor em RGB565. Use a macro `RGB565(r,g,b)` para converter.
     *
     * @return Referência a este objeto (permite encadeamento).
     *
     * @par Exemplo
     * @code
     * btn.corFundo(0x001F);        // azul puro
     * btn.corFundo(RGB565(0,100,200)); // usando macro
     * @endcode
     */
    NexObjeto& corFundo(uint32_t cor);

    /**
     * @brief Define a cor do texto (atributo `pco`).
     *
     * @param cor  Cor em RGB565.
     * @return Referência a este objeto.
     *
     * @par Exemplo
     * @code
     * txt.corTexto(65535);   // branco
     * txt.corTexto(0xF800);  // vermelho
     * @endcode
     */
    NexObjeto& corTexto(uint32_t cor);

    /**
     * @brief Controla a visibilidade do componente (comando `vis`).
     *
     * @param mostrar  true = mostrar, false = ocultar.
     * @return Referência a este objeto.
     *
     * @par Exemplo
     * @code
     * btn.visivel(false);  // oculta o botão
     * btn.visivel(true);   // mostra novamente
     * @endcode
     */
    NexObjeto& visivel(bool mostrar);

    /**
     * @brief Força o redesenho do componente (comando `ref`).
     *
     * @details
     * Útil após mudar múltiplos atributos e querer atualizar o display
     * uma única vez ao final. Os métodos `set()` já chamam `ref`
     * internamente; use este método apenas quando necessário.
     *
     * @return Referência a este objeto.
     */
    NexObjeto& atualizar();

    /**
     * @brief Envia um comando genérico ao Nextion (acesso de baixo nível).
     *
     * @details
     * Use quando precisar de um comando que não tem método específico.
     * O terminador `0xFF 0xFF 0xFF` é adicionado automaticamente.
     *
     * @param cmd  Comando a enviar.
     * @return Referência a este objeto.
     *
     * @par Exemplo
     * @code
     * btn.cmd("click b0,1");     // simula toque no botão b0
     * comp.cmd("cirs 100,100,30,RED");  // desenha círculo
     * @endcode
     */
    NexObjeto& cmd(const char* cmd);

    // -----------------------------------------------------------------------
    // Informações do objeto
    // -----------------------------------------------------------------------

    /** @brief Retorna o ID da página deste componente. */
    uint8_t     pagina()  const { return _pid; }

    /** @brief Retorna o ID do componente. */
    uint8_t     id()      const { return _cid; }

    /** @brief Retorna o nome (objname) do componente. */
    const char* nome()    const { return _nome; }

    /**
     * @brief Imprime informações do objeto (debug).
     *
     * @details Só produz saída se debug estiver habilitado.
     */
    void imprimirInfo() const;

protected:
    uint8_t     _pid;
    uint8_t     _cid;
    const char* _nome;
};

// ---------------------------------------------------------------------------
// Macro auxiliar para converter RGB (0-255 cada) para RGB565
// ---------------------------------------------------------------------------

/**
 * @brief Converte componentes RGB para o formato RGB565 usado pelo Nextion.
 *
 * @param r  Vermelho (0–255).
 * @param g  Verde    (0–255).
 * @param b  Azul     (0–255).
 *
 * @return Valor uint32_t no formato RGB565.
 *
 * @par Exemplo
 * @code
 * btn.corFundo(RGB565(30, 150, 255));  // azul claro
 * txt.corTexto(RGB565(255, 255, 255)); // branco
 * @endcode
 */
#define RGB565(r, g, b) \
    ((uint32_t)(((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

// Cores pré-definidas em RGB565
#define COR_PRETO    0x0000u
#define COR_BRANCO   0xFFFFu
#define COR_VERMELHO 0xF800u
#define COR_VERDE    0x07E0u
#define COR_AZUL     0x001Fu
#define COR_AMARELO  0xFFE0u
#define COR_CIANO    0x07FFu
#define COR_MAGENTA  0xF81Fu
#define COR_CINZA    0x7BEFu

#endif /* __NEXOBJETO_H__ */
