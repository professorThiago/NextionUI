/**
 * @file NexComponentes.h
 * @brief Todos os componentes Nextion com API em português.
 *
 * @details
 * Todos os componentes herdam de `NexObjeto` ou `NexToque` e ganham
 * **gratuitamente** os métodos `set()`, `get()`, `setf()`, `corFundo()`,
 * `corTexto()`, `visivel()` e `atualizar()`.
 *
 * Cada componente adiciona apenas os métodos **específicos** ao seu tipo,
 * sem duplicar o código da classe base.
 *
 * ### Hierarquia
 *
 * ```
 * NexObjeto
 *   ├── NexToque
 *   │     ├── NexBotao          (Button)
 *   │     ├── NexTexto          (Text)
 *   │     ├── NexNumero         (Number)
 *   │     ├── NexSlider         (Slider)
 *   │     ├── NexCheckbox       (Checkbox)
 *   │     ├── NexRadio          (Radio)
 *   │     ├── NexBotaoDuplo     (DualStateButton)
 *   │     ├── NexVariavel       (Variable)
 *   │     ├── NexTimer          (Timer)
 *   │     ├── NexTextoDeslizante(Scrolltext)
 *   │     ├── NexImagem         (Picture)
 *   │     ├── NexCrop           (Crop)
 *   │     ├── NexHotspot        (Hotspot)
 *   │     └── NexPagina         (Page)
 *   └── NexObjeto direto
 *         ├── NexBarraProgresso (ProgressBar)
 *         ├── NexGrafico        (Waveform)
 *         └── NexGauge          (Gauge)
 * ```
 *
 * @author  professorThiago
 * @version 1.0.0
 * @license MIT
 */

#ifndef __NEXCOMPONENTES_H__
#define __NEXCOMPONENTES_H__

#include "NexToque.h"

// =============================================================================
// NexBotao — Botão (Button)
// =============================================================================

/**
 * @brief Componente botão do Nextion.
 *
 * @details
 * Além dos métodos herdados (`set`, `get`, `corFundo`, `corTexto`, etc.),
 * o botão adiciona atalhos para o texto e cores de pressionado.
 *
 * @par Uso típico
 * @code
 * NexBotao btnLigar(0, 2, "btn_ligar");
 *
 * void setup() {
 *     btnLigar.texto("LIGAR");
 *     btnLigar.corFundo(RGB565(0, 100, 200));
 *     btnLigar.aoSoltar([]() { ligar(); });
 * }
 * @endcode
 */
class NexBotao : public NexToque {
public:
    NexBotao(uint8_t pid, uint8_t cid, const char* nome)
        : NexToque(pid, cid, nome) {}

    /** @brief Define o texto do botão. Atalho para `set("txt", texto)`. */
    NexBotao& texto(const char* t)        { set("txt", t);    return *this; }

    /** @brief Lê o texto do botão. */
    uint16_t  texto(char* buf, uint16_t n){ return get("txt", buf, n); }

    /** @brief Cor de fundo quando pressionado (atributo `bco2`). */
    NexBotao& corFundoPressionado(uint32_t cor) { return (NexBotao&)set("bco2", cor); }

    /** @brief Cor do texto quando pressionado (atributo `pco2`). */
    NexBotao& corTextoPressionado(uint32_t cor) { return (NexBotao&)set("pco2", cor); }
};

// =============================================================================
// NexTexto — Texto (Text)
// =============================================================================

/**
 * @brief Componente de texto do Nextion.
 *
 * @par Uso típico
 * @code
 * NexTexto tTemp(1, 3, "t_temp");
 *
 * // Atualizar temperatura com formatação
 * tTemp.setf("txt", "%d°C", temperatura);
 *
 * // Mudar cor conforme valor
 * tTemp.corTexto(temperatura > 30 ? COR_VERMELHO : COR_BRANCO);
 * @endcode
 */
class NexTexto : public NexToque {
public:
    NexTexto(uint8_t pid, uint8_t cid, const char* nome)
        : NexToque(pid, cid, nome) {}

    /** @brief Define o texto. Atalho para `set("txt", texto)`. */
    NexTexto& texto(const char* t)         { set("txt", t);    return *this; }

    /** @brief Lê o texto. */
    uint16_t  texto(char* buf, uint16_t n) { return get("txt", buf, n); }
};

// =============================================================================
// NexNumero — Número (Number)
// =============================================================================

/**
 * @brief Componente de número do Nextion.
 *
 * @par Uso típico
 * @code
 * NexNumero nContador(0, 4, "n_cnt");
 * nContador.valor(42);
 *
 * uint32_t v;
 * nContador.valor(v);
 * @endcode
 */
class NexNumero : public NexToque {
public:
    NexNumero(uint8_t pid, uint8_t cid, const char* nome)
        : NexToque(pid, cid, nome) {}

    /** @brief Define o valor numérico. Atalho para `set("val", v)`. */
    NexNumero& valor(uint32_t v)        { set("val", v); return *this; }

    /** @brief Lê o valor numérico. */
    bool       valor(uint32_t& dest)    { return get("val", dest); }
};

// =============================================================================
// NexSlider — Controle deslizante (Slider)
// =============================================================================

/**
 * @brief Componente slider do Nextion.
 *
 * @par Uso típico
 * @code
 * NexSlider slVolume(0, 5, "sl_vol");
 * slVolume.minimo(0).maximo(100).valor(50);
 *
 * slVolume.aoSoltar([]() {
 *     uint32_t v;
 *     slVolume.valor(v);
 *     Serial.println("Volume: " + String(v));
 * });
 * @endcode
 */
class NexSlider : public NexToque {
public:
    NexSlider(uint8_t pid, uint8_t cid, const char* nome)
        : NexToque(pid, cid, nome) {}

    /** @brief Define o valor atual do slider. */
    NexSlider& valor(uint32_t v)     { set("val", v);    return *this; }

    /** @brief Lê o valor atual do slider. */
    bool       valor(uint32_t& dest) { return get("val", dest); }

    /** @brief Define o valor mínimo. */
    NexSlider& minimo(uint32_t v)    { set("minval", v); return *this; }

    /** @brief Define o valor máximo. */
    NexSlider& maximo(uint32_t v)    { set("maxval", v); return *this; }
};

// =============================================================================
// NexBarraProgresso — Barra de progresso (Progress Bar)
// =============================================================================

/**
 * @brief Componente barra de progresso do Nextion.
 *
 * @details
 * Não suporta toque. Herda diretamente de `NexObjeto`.
 *
 * @par Uso típico
 * @code
 * NexBarraProgresso barScan(5, 3, "j_scan");
 * barScan.valor(0);
 *
 * // Durante o scan, atualiza a cada módulo encontrado:
 * barScan.valor(porcentagem);
 * @endcode
 */
class NexBarraProgresso : public NexObjeto {
public:
    NexBarraProgresso(uint8_t pid, uint8_t cid, const char* nome)
        : NexObjeto(pid, cid, nome) {}

    /** @brief Define o valor da barra (0–100). */
    NexBarraProgresso& valor(uint32_t v)     { set("val", v); return *this; }

    /** @brief Lê o valor atual. */
    bool               valor(uint32_t& dest) { return get("val", dest); }
};

// =============================================================================
// NexGrafico — Gráfico de forma de onda (Waveform)
// =============================================================================

/**
 * @brief Componente de gráfico do Nextion.
 *
 * @par Uso típico
 * @code
 * NexGrafico grafico(1, 2, "s0");
 *
 * // Adiciona ponto no canal 0
 * grafico.adicionarPonto(0, analogRead(34) / 16);  // 0-255
 * @endcode
 */
class NexGrafico : public NexObjeto {
public:
    NexGrafico(uint8_t pid, uint8_t cid, const char* nome)
        : NexObjeto(pid, cid, nome) {}

    /**
     * @brief Adiciona um ponto de dados ao gráfico.
     *
     * @param canal  Canal (0–3).
     * @param valor  Valor do ponto (0–255).
     *
     * @return true se enviado com sucesso.
     */
    bool adicionarPonto(uint8_t canal, uint8_t valor)
    {
        if (canal > 3) return false;
        nexEnviarCmdF("add %u,%u,%u", _cid, canal, valor);
        return true;
    }
};

// =============================================================================
// NexGauge — Mostrador analógico (Gauge)
// =============================================================================

/**
 * @brief Componente gauge (mostrador circular) do Nextion.
 *
 * @par Uso típico
 * @code
 * NexGauge gTemp(1, 6, "z0");
 * gTemp.valor(180);  // 0–360 graus
 * @endcode
 */
class NexGauge : public NexObjeto {
public:
    NexGauge(uint8_t pid, uint8_t cid, const char* nome)
        : NexObjeto(pid, cid, nome) {}

    /** @brief Define o ângulo do ponteiro (0–360). */
    NexGauge& valor(uint32_t v)     { set("val", v); return *this; }

    /** @brief Lê o ângulo atual. */
    bool      valor(uint32_t& dest) { return get("val", dest); }
};

// =============================================================================
// NexCheckbox — Caixa de seleção (Checkbox)
// =============================================================================

/**
 * @brief Componente checkbox do Nextion.
 *
 * @par Uso típico
 * @code
 * NexCheckbox chkAtivo(0, 7, "cb_ativo");
 * chkAtivo.marcado(true);
 *
 * chkAtivo.aoSoltar([]() {
 *     uint32_t v;
 *     chkAtivo.valor(v);
 *     Serial.println(v ? "Marcado" : "Desmarcado");
 * });
 * @endcode
 */
class NexCheckbox : public NexToque {
public:
    NexCheckbox(uint8_t pid, uint8_t cid, const char* nome)
        : NexToque(pid, cid, nome) {}

    /** @brief Define se está marcado (1) ou desmarcado (0). */
    NexCheckbox& marcado(bool v)       { set("val", v ? 1 : 0); return *this; }

    /** @brief Lê se está marcado. */
    bool         marcado(bool& dest)   {
        uint32_t v;
        bool ok = get("val", v);
        dest = (v != 0);
        return ok;
    }
};

// =============================================================================
// NexRadio — Botão de rádio (Radio)
// =============================================================================

/**
 * @brief Componente radio button do Nextion.
 */
class NexRadio : public NexToque {
public:
    NexRadio(uint8_t pid, uint8_t cid, const char* nome)
        : NexToque(pid, cid, nome) {}

    /** @brief Define o estado selecionado (1) ou não (0). */
    NexRadio& selecionado(bool v)   { set("val", v ? 1 : 0); return *this; }

    /** @brief Lê se está selecionado. */
    bool      valor(uint32_t& dest) { return get("val", dest); }
};

// =============================================================================
// NexBotaoDuplo — Botão de dois estados (DualStateButton)
// =============================================================================

/**
 * @brief Componente botão de dois estados (toggle) do Nextion.
 *
 * @par Uso típico
 * @code
 * NexBotaoDuplo btnToggle(0, 8, "bt0");
 *
 * btnToggle.aoSoltar([]() {
 *     uint32_t estado;
 *     btnToggle.valor(estado);
 *     Serial.println(estado ? "ON" : "OFF");
 * });
 * @endcode
 */
class NexBotaoDuplo : public NexToque {
public:
    NexBotaoDuplo(uint8_t pid, uint8_t cid, const char* nome)
        : NexToque(pid, cid, nome) {}

    /** @brief Define o estado (0 ou 1). */
    NexBotaoDuplo& valor(uint32_t v)     { set("val", v); return *this; }

    /** @brief Lê o estado atual. */
    bool           valor(uint32_t& dest) { return get("val", dest); }

    /** @brief Define o texto. */
    NexBotaoDuplo& texto(const char* t)  { set("txt", t); return *this; }
};

// =============================================================================
// NexVariavel — Variável interna (Variable)
// =============================================================================

/**
 * @brief Variável interna do Nextion (sem representação visual).
 *
 * @details
 * Útil para guardar estado no próprio display ou comunicar valores
 * entre páginas do Nextion.
 *
 * @par Uso típico
 * @code
 * NexVariavel varSala(0, 10, "va_sala");
 * varSala.valor(3);  // armazena o número da sala
 *
 * uint32_t sala;
 * varSala.valor(sala);  // lê de volta
 * @endcode
 */
class NexVariavel : public NexToque {
public:
    NexVariavel(uint8_t pid, uint8_t cid, const char* nome)
        : NexToque(pid, cid, nome) {}

    /** @brief Define valor numérico. */
    NexVariavel& valor(uint32_t v)     { set("val", v); return *this; }

    /** @brief Lê valor numérico. */
    bool         valor(uint32_t& dest) { return get("val", dest); }

    /** @brief Define valor de texto. */
    NexVariavel& texto(const char* t)         { set("txt", t); return *this; }

    /** @brief Lê valor de texto. */
    uint16_t     texto(char* buf, uint16_t n) { return get("txt", buf, n); }
};

// =============================================================================
// NexTimer — Timer (Timer)
// =============================================================================

/**
 * @brief Componente timer do Nextion.
 *
 * @details
 * O callback é disparado periodicamente com o intervalo definido em `ciclo()`.
 * O intervalo mínimo é 50ms.
 *
 * @par Uso típico
 * @code
 * NexTimer tmRelogio(0, 11, "tm0");
 *
 * void setup() {
 *     tmRelogio.ciclo(1000);   // dispara a cada 1s
 *     tmRelogio.iniciar();
 *     tmRelogio.aoDisparar([]() {
 *         // atualiza relógio
 *     });
 * }
 * @endcode
 */
class NexTimer : public NexToque {
public:
    NexTimer(uint8_t pid, uint8_t cid, const char* nome)
        : NexToque(pid, cid, nome) {}

    /**
     * @brief Define o intervalo do timer em ms (mínimo: 50ms).
     *
     * @param ms  Intervalo em milissegundos.
     */
    NexTimer& ciclo(uint32_t ms)
    {
        if (ms < 50) ms = 50;
        set("tim", ms);
        return *this;
    }

    /** @brief Lê o intervalo atual. */
    bool ciclo(uint32_t& dest) { return get("tim", dest); }

    /** @brief Habilita o timer. */
    NexTimer& iniciar()  { set("en", (uint32_t)1); return *this; }

    /** @brief Desabilita o timer. */
    NexTimer& parar()    { set("en", (uint32_t)0); return *this; }

    /**
     * @brief Registra callback disparado quando o timer expira.
     *
     * @details Internamente usa `aoSoltar()` (evento pop do Nextion).
     *
     * @param cb  Função `void cb()`.
     */
    void aoDisparar(CbToqueSimples cb) { aoSoltar(cb); }
};

// =============================================================================
// NexTextoDeslizante — Texto com rolagem (Scrolltext)
// =============================================================================

/**
 * @brief Componente de texto com rolagem automática do Nextion.
 *
 * @par Uso típico
 * @code
 * NexTextoDeslizante txtAviso(6, 2, "x0");
 * txtAviso.texto("Reunião às 14h na sala de professores")
 *         .ciclo(200)
 *         .iniciar();
 * @endcode
 */
class NexTextoDeslizante : public NexToque {
public:
    NexTextoDeslizante(uint8_t pid, uint8_t cid, const char* nome)
        : NexToque(pid, cid, nome) {}

    /** @brief Define o texto. */
    NexTextoDeslizante& texto(const char* t) { set("txt", t); return *this; }

    /** @brief Define o intervalo de rolagem (ms). */
    NexTextoDeslizante& ciclo(uint32_t ms)
    {
        if (ms < 8) ms = 8;
        set("tim", ms);
        return *this;
    }

    /** @brief Inicia a rolagem. */
    NexTextoDeslizante& iniciar() { set("en", (uint32_t)1); return *this; }

    /** @brief Para a rolagem. */
    NexTextoDeslizante& parar()   { set("en", (uint32_t)0); return *this; }
};

// =============================================================================
// NexImagem — Imagem (Picture)
// =============================================================================

/**
 * @brief Componente de imagem do Nextion.
 *
 * @par Uso típico
 * @code
 * NexImagem imgStatus(1, 7, "p0");
 * imgStatus.imagem(3);   // exibe a imagem de índice 3
 * @endcode
 */
class NexImagem : public NexToque {
public:
    NexImagem(uint8_t pid, uint8_t cid, const char* nome)
        : NexToque(pid, cid, nome) {}

    /** @brief Define o índice da imagem a exibir. */
    NexImagem& imagem(uint32_t idx)     { set("pic", idx); return *this; }

    /** @brief Lê o índice atual. */
    bool       imagem(uint32_t& dest)   { return get("pic", dest); }
};

// =============================================================================
// NexCrop — Imagem recortada (Crop)
// =============================================================================

/**
 * @brief Componente de imagem com recorte do Nextion.
 */
class NexCrop : public NexToque {
public:
    NexCrop(uint8_t pid, uint8_t cid, const char* nome)
        : NexToque(pid, cid, nome) {}

    /** @brief Define o índice da imagem recortada. */
    NexCrop& imagem(uint32_t idx)   { set("picc", idx); return *this; }

    /** @brief Lê o índice atual. */
    bool     imagem(uint32_t& dest) { return get("picc", dest); }
};

// =============================================================================
// NexHotspot — Área de toque invisível (Hotspot)
// =============================================================================

/**
 * @brief Área de toque invisível do Nextion.
 *
 * @details
 * Útil para criar regiões clicáveis sobre imagens ou elementos sem
 * componente de toque.
 */
class NexHotspot : public NexToque {
public:
    NexHotspot(uint8_t pid, uint8_t cid, const char* nome)
        : NexToque(pid, cid, nome) {}
};

// =============================================================================
// NexPagina — Página (Page)
// =============================================================================

/**
 * @brief Representa uma página do display Nextion.
 *
 * @details
 * Além de poder registrar callbacks (disparados quando a página é
 * exibida), oferece o método `mostrar()` para navegar até ela.
 *
 * @par Uso típico
 * @code
 * NexPagina p0(0, 0, "p0_sleep");
 * NexPagina p1(1, 0, "p1_dash");
 *
 * p1.aoEntrar([]() { Serial.println("Dashboard aberto"); });
 *
 * // Navegar para o dashboard:
 * p1.mostrar();
 * @endcode
 */
class NexPagina : public NexToque {
public:
    NexPagina(uint8_t pid, uint8_t cid, const char* nome)
        : NexToque(pid, cid, nome) {}

    /**
     * @brief Navega para esta página.
     *
     * @return true se o comando foi enviado.
     */
    bool mostrar()
    {
        nexEnviarCmdF("page %s", _nome);
        return nexAguardarOk();
    }

    /**
     * @brief Registra callback chamado ao entrar na página.
     *
     * @details Internamente usa `aoPressionar()`.
     *
     * @param cb  Função `void cb()`.
     */
    void aoEntrar(CbToqueSimples cb) { aoPressionar(cb); }
};

// =============================================================================
// NexRtc — Relógio em tempo real (RTC) do Nextion
// =============================================================================

/**
 * @brief Interface com o RTC interno do Nextion.
 *
 * @details
 * O Nextion possui um RTC interno (sem bateria de backup). Utilize
 * `escreverHora()` para sincronizar via NTP ou RTC externo no boot.
 *
 * @par Uso típico
 * @code
 * NexRtc rtc;
 *
 * // Sincronizar com NTP (após getLocalTime)
 * struct tm ti;
 * if (getLocalTime(&ti)) {
 *     rtc.escrever(ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday,
 *                  ti.tm_hour, ti.tm_min, ti.tm_sec);
 * }
 *
 * // Ler a hora atual
 * uint32_t h, m, s;
 * rtc.lerHora(h, m, s);
 * @endcode
 */
class NexRtc {
public:
    /**
     * @brief Escreve data e hora completa no RTC do Nextion.
     *
     * @param ano   Ano (ex: 2025).
     * @param mes   Mês (1–12).
     * @param dia   Dia (1–31).
     * @param hora  Hora (0–23).
     * @param min   Minuto (0–59).
     * @param seg   Segundo (0–59).
     */
    void escrever(uint16_t ano, uint8_t mes, uint8_t dia,
                  uint8_t hora, uint8_t min, uint8_t seg)
    {
        nexEnviarCmdF("rtc0=%u", ano);
        nexEnviarCmdF("rtc1=%u", mes);
        nexEnviarCmdF("rtc2=%u", dia);
        nexEnviarCmdF("rtc3=%u", hora);
        nexEnviarCmdF("rtc4=%u", min);
        nexEnviarCmdF("rtc5=%u", seg);
    }

    /**
     * @brief Lê um campo específico do RTC.
     *
     * @param campo  "ano","mes","dia","hora","min","seg","semana".
     * @param dest   Variável destino.
     *
     * @return true se lido com sucesso.
     */
    bool ler(const char* campo, uint32_t& dest)
    {
        const char* campos[] = {"ano","mes","dia","hora","min","seg","semana",nullptr};
        const char* rtcIds[] = {"0","1","2","3","4","5","6"};
        for (uint8_t i = 0; campos[i]; i++) {
            if (strcmp(campo, campos[i]) == 0) {
                nexEnviarCmdF("get rtc%s", rtcIds[i]);
                return nexLerNumero(&dest);
            }
        }
        return false;
    }

    /**
     * @brief Lê hora, minuto e segundo de uma vez.
     *
     * @param hora  Variável para a hora.
     * @param min   Variável para o minuto.
     * @param seg   Variável para o segundo.
     *
     * @return true se todos os campos foram lidos.
     */
    bool lerHora(uint32_t& hora, uint32_t& min, uint32_t& seg)
    {
        return ler("hora", hora) && ler("min", min) && ler("seg", seg);
    }
};

// =============================================================================
// NexGpio — GPIO do Nextion
// =============================================================================

/**
 * @brief Interface com os pinos GPIO do próprio display Nextion.
 *
 * @par Uso típico
 * @code
 * NexGpio gpio;
 * gpio.modoSaida(1);      // GPIO 1 como saída push-pull
 * gpio.escrever(1, HIGH); // seta GPIO 1 alto
 * @endcode
 */
class NexGpio {
public:
    /**
     * @brief Configura um pino GPIO do Nextion.
     *
     * @param pino  Número do pino (0–7).
     * @param modo  0=entrada pull-up, 1=entrada com binding,
     *              2=saída push-pull, 3=PWM, 4=open-drain.
     * @param bindId  ID do componente vinculado (modo 1).
     */
    bool configurar(uint32_t pino, uint32_t modo, uint32_t bindId = 0)
    {
        nexEnviarCmdF("cfgpio %lu,%lu,%lu", pino, modo, bindId);
        return nexAguardarOk();
    }

    /** @brief Atalho: configura como saída. */
    bool modoSaida(uint32_t pino) { return configurar(pino, 2); }

    /** @brief Atalho: configura como entrada. */
    bool modoEntrada(uint32_t pino) { return configurar(pino, 0); }

    /** @brief Escreve HIGH (1) ou LOW (0) em um pino GPIO. */
    bool escrever(uint32_t pino, uint32_t valor)
    {
        nexEnviarCmdF("pio%lu=%lu", pino, valor);
        return nexAguardarOk();
    }

    /** @brief Lê o estado de um pino GPIO. */
    bool ler(uint32_t pino, uint32_t& dest)
    {
        nexEnviarCmdF("get pio%lu", pino);
        return nexLerNumero(&dest);
    }

    /** @brief Escreve valor PWM (0–100%). */
    bool pwm(uint32_t pino, uint32_t duty)
    {
        nexEnviarCmdF("pwm%lu=%lu", pino, duty);
        return nexAguardarOk();
    }
};

#endif /* __NEXCOMPONENTES_H__ */
