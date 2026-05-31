# NextionUI

Biblioteca moderna para displays **Nextion** no ESP32 — API em português, zero alocações dinâmicas, fluent interface e Doxygen completo.

[![Plataforma](https://img.shields.io/badge/plataforma-ESP32%20%7C%20ESP8266%20%7C%20Arduino-blue)](https://platformio.org)
[![Licença](https://img.shields.io/badge/licença-MIT-green)](LICENSE)
[![Versão](https://img.shields.io/badge/versão-1.0.0-orange)](library.json)

---

## Por que esta biblioteca?

A ITEADLIB_Arduino_Nextion original tem dois problemas sérios:

| Problema | Esta biblioteca |
|---|---|
| ~400 linhas duplicadas (get/set por componente) | Método genérico `set(attr, val)` na base |
| `String` do Arduino em todo envio (heap fragmentado) | Buffer fixo `snprintf` — zero alocações |
| Funções globais espalhadas | Classe `NexDisplay` como ponto único |
| Sem suporte a lambda | `btn.aoSoltar([]() { ... })` |
| Debug sempre ligado (ruído no Serial) | Debug desativado por padrão |
| Documentação em inglês incompleta | Doxygen completo em português |

---

## Instalação

```ini
; platformio.ini
lib_deps =
    https://github.com/professorThiago/NextionUI
```

---

## Início rápido

```cpp
#include <NextionUI.h>

NexDisplay display;
NexBotao   btnLigar(0, 2, "btn_ligar");
NexTexto   tStatus (0, 3, "t_status");

void setup() {
    display.begin(Serial2, 115200, 16, 17);
    display.escutar(btnLigar);

    btnLigar.aoSoltar([]() {
        tStatus.texto("LIGADO")
               .corTexto(COR_BRANCO)
               .corFundo(RGB565(0, 150, 0));
    });
}

void loop() {
    display.atualizar();
}
```

---

## API principal

### NexDisplay

```cpp
display.begin(Serial2, 115200, 16, 17);  // inicializa
display.atualizar();                       // no loop()
display.escutar(componente);               // registra toque
display.irPara("p1_dash");                 // navega por nome
display.irPara(1);                         // navega por índice
display.paginaAtual();                     // retorna uint8_t
display.brilho(80);                        // 0–100%
display.repouso(300);                      // segundos (0=nunca)
display.cmd("dim=50");                     // comando bruto
display.cmdf("t0.txt=\"%d°C\"", 22);      // printf-style
display.aoMudarPagina([](uint8_t pg) { }); // callback de página
```

### Atualizar componentes

```cpp
// Texto simples
txt.texto("Olá");

// Texto formatado
txt.setf("txt", "%d°C", temperatura);
txt.setf("txt", "%02d:%02d", hora, minuto);
txt.setf("txt", "%.1f%%", umidade);

// Qualquer atributo numérico pelo nome
comp.set("bco", RGB565(0, 100, 200));  // cor de fundo
comp.set("pco", COR_BRANCO);           // cor do texto
comp.set("val", 42);                   // valor
comp.set("en",  1);                    // habilitado

// Ler atributo numérico
uint32_t v;
comp.get("val", v);

// Ler atributo de texto
char buf[32];
comp.get("txt", buf, sizeof(buf));
```

### Fluent interface (encadeamento)

```cpp
btn.texto("ATIVO")
   .corFundo(RGB565(0, 120, 200))
   .corTexto(COR_BRANCO)
   .visivel(true)
   .atualizar();
```

### Callbacks

```cpp
// Lambda simples (recomendado)
btn.aoPressionar([]() { Serial.println("Pressionado!"); });
btn.aoSoltar    ([]() { Serial.println("Solto!"); });

// Com ponteiro de contexto
btn.aoSoltar(minhaFuncao, &meuObjeto);

// Timer
timer.ciclo(1000).iniciar();
timer.aoDisparar([]() { atualizarDados(); });
```

---

## Cores em RGB565

```cpp
RGB565(r, g, b)     // converte RGB 0-255 para RGB565

COR_PRETO    COR_BRANCO   COR_VERMELHO
COR_VERDE    COR_AZUL     COR_AMARELO
COR_CIANO    COR_MAGENTA  COR_CINZA
```

---

## Debug

```cpp
// Opção 1 — DebugManager (recomendado)
#include <DebugManager.h>   // ANTES de NextionUI.h
#include <NextionUI.h>
// Mensagens aparecem com prefixo [Nextion] no nível VERBOSE

// Opção 2 — Serial direto
#define NEX_DEBUG_SERIAL Serial
#include <NextionUI.h>
```

---

## Componentes disponíveis

| Classe | Nextion | Principais métodos |
|---|---|---|
| `NexBotao` | Button | `texto()`, `corFundoPressionado()` |
| `NexTexto` | Text | `texto()`, `setf()` |
| `NexNumero` | Number | `valor()` |
| `NexSlider` | Slider | `valor()`, `minimo()`, `maximo()` |
| `NexBarraProgresso` | ProgressBar | `valor()` |
| `NexGrafico` | Waveform | `adicionarPonto(canal, val)` |
| `NexGauge` | Gauge | `valor()` |
| `NexCheckbox` | Checkbox | `marcado()` |
| `NexRadio` | Radio | `selecionado()`, `valor()` |
| `NexBotaoDuplo` | DualStateButton | `valor()`, `texto()` |
| `NexVariavel` | Variable | `valor()`, `texto()` |
| `NexTimer` | Timer | `ciclo()`, `iniciar()`, `parar()`, `aoDisparar()` |
| `NexTextoDeslizante` | Scrolltext | `texto()`, `ciclo()`, `iniciar()` |
| `NexImagem` | Picture | `imagem()` |
| `NexCrop` | Crop | `imagem()` |
| `NexHotspot` | Hotspot | `aoPressionar()`, `aoSoltar()` |
| `NexPagina` | Page | `mostrar()`, `aoEntrar()` |
| `NexRtc` | RTC | `escrever()`, `ler()`, `lerHora()` |
| `NexGpio` | GPIO | `modoSaida()`, `escrever()`, `ler()`, `pwm()` |

---

## Configuração

```cpp
#define NEX_BUFFER_CMD        256   // buffer (padrão: 128)
#define NEX_MAX_COMPONENTES    64   // lista de escuta (padrão: 32)
#define NEX_TIMEOUT_MS        200   // timeout serial (padrão: 100)
#define NEX_AGUARDAR_CONFIRMACAO 1  // aguardar ACK (padrão: 0)
#include <NextionUI.h>
```

---

## Exemplos incluídos

| Exemplo | Descrição |
|---|---|
| `01_OlaMundo` | Inicializar display e atualizar texto |
| `02_BotaoCallback` | Lambdas, callbacks estilo antigo, toggle |
| `03_AtualizarComponentes` | Todos os métodos de atualização |
| `04_NexDisplay` | Sistema multipágina com timer e slider |
| `05_MixCompleto` | Gauge, gráfico, RTC, GPIO, texto deslizante |
| `06_DebugManager` | Integração com DebugManager |

---

## Licença

MIT — veja [LICENSE](LICENSE)

## Autor

**professorThiago** — [github.com/professorThiago](https://github.com/professorThiago)
