# NextionUI

Biblioteca moderna para displays Nextion no ESP32 — API em português, zero alocações dinâmicas, integração com DebugManager, NexDisplay como ponto de entrada único, fluent interface e Doxygen completo.

Inclui **NexAtualizador** para atualização remota do firmware (.tft) do display via serial.

[![Plataforma](https://img.shields.io/badge/plataforma-ESP32%20%7C%20ESP8266%20%7C%20AVR-blue)](https://platformio.org)
[![Licença](https://img.shields.io/badge/licença-MIT-green)](LICENSE)
[![Versão](https://img.shields.io/badge/versão-1.1.0-orange)](library.json)

---

## Funcionalidades

| Recurso | Descrição |
|---|---|
| NexDisplay | Ponto de entrada único — gerencia serial, polling e componentes |
| set() genérico | `set("txt", "valor")`, `set("val", 42)`, `set("pco", COR_VERDE)` |
| Callbacks via lambda | `botao.aoTocar([](){ /* ... */ });` |
| Buffer fixo (snprintf) | Zero uso de `String` do Arduino — sem fragmentação de heap |
| Macro RGB565 | `RGB565(r, g, b)` + constantes `COR_*` pré-definidas |
| Fluent interface | `display.pagina("dash").enviar("t1.txt=\"Olá\"")` |
| Debug integrado | DebugManager opcional via `__has_include` |
| NexAtualizador | Upload de .tft via serial — streaming HTTP, SD ou SPIFFS |

---

## Instalação

```ini
; platformio.ini
lib_deps =
    https://github.com/professorThiago/NextionUI
```

---

## Uso rápido — Display

```cpp
#include <NextionUI.h>

NexDisplay display(Serial2, 921600, 16, 17); // serial, baud, RX, TX

NexBotao btnPower(1, 3, "btnPower");
NexTexto txtTemp(1, 5, "txtTemp");

void setup() {
    display.begin();
    display.registrar(btnPower);
    display.registrar(txtTemp);

    btnPower.aoTocar([]() {
        txtTemp.set("txt", "Ligado!");
        txtTemp.set("pco", COR_VERDE);
    });
}

void loop() {
    display.atualizar();
}
```

---

## Componentes disponíveis

| Classe | Nextion | Descrição |
|---|---|---|
| NexBotao | Button | Toque simples |
| NexBotaoDuplo | Dual-state Button | Liga/desliga |
| NexTexto | Text | Texto estático |
| NexNumero | Number | Valor numérico |
| NexSlider | Slider | Barra deslizante |
| NexCheckbox | Checkbox | Caixa de seleção |
| NexRadio | Radio | Botão de rádio |
| NexVariavel | Variable | Variável interna |
| NexTimer | Timer | Temporizador |
| NexTextoDeslizante | Scrolling Text | Texto rolante |
| NexImagem | Picture | Imagem por ID |
| NexCrop | Crop | Imagem recortada |
| NexHotspot | Hotspot | Área tocável invisível |
| NexPagina | Page | Navegação de páginas |

Todos herdam de `NexObjeto` e suportam `set(atributo, valor)` genérico.

---

## API — set() genérico

```cpp
// Texto
txtStatus.set("txt", "Conectado");

// Número
numTemp.set("val", 22);

// Cor (foreground, background, press color)
txtStatus.set("pco", COR_VERDE);
txtStatus.set("bco", RGB565(13, 17, 23));

// Visibilidade
display.enviar("vis btnConfig,1");  // mostrar
display.enviar("vis btnConfig,0");  // esconder

// Navegação
display.pagina("dash");
```

---

## Cores pré-definidas

```cpp
COR_BRANCO, COR_PRETO, COR_VERMELHO, COR_VERDE, COR_AZUL,
COR_AMARELO, COR_CIANO, COR_MAGENTA, COR_LARANJA,
COR_CINZA_CLARO, COR_CINZA_ESCURO

// Cor customizada via macro RGB565
uint16_t minhaCor = RGB565(30, 144, 255); // azul dodger
```

---

## NexAtualizador — Atualização remota do display

O `NexAtualizador` permite atualizar o firmware (.tft) do Nextion via ESP32, sem cartão SD. O ESP32 baixa o arquivo de um servidor HTTP e envia para o display via serial usando o protocolo de upload nativo.

### Uso com HTTP

```cpp
#include <NextionUI.h>
#include <NexAtualizador.h>
#include <HTTPClient.h>

NexAtualizador nexOta(Serial2);

void atualizarTela(const char* url) {
    nexOta.definirBaudOperacao(921600);  // baud atual do display
    nexOta.definirBaudUpload(115200);    // baud para transferência

    nexOta.aoProgresso([](uint8_t pct) {
        Serial.printf("Nextion: %d%%\n", pct);
    });

    HTTPClient http;
    http.begin(url);
    int codigo = http.GET();
    if (codigo != 200) { http.end(); return; }

    uint32_t tamanho = http.getSize();
    nexOta.atualizar(*http.getStreamPtr(), tamanho);
    http.end();
}
```

### Uso com cartão SD

```cpp
#include <NexAtualizador.h>
#include <SD.h>

NexAtualizador nexOta(Serial2);

void atualizarTelaSD() {
    File arquivo = SD.open("/interface.tft");
    if (!arquivo) return;

    nexOta.definirBaudOperacao(921600);
    nexOta.atualizar(arquivo, arquivo.size());
    arquivo.close();
}
```

### Protocolo de upload

```
ESP32 envia: "whmi-wri <tamanho>,<baud>,0" + 0xFF 0xFF 0xFF
Nextion responde: 0x05 (ACK)
ESP32 envia: blocos de 4096 bytes
Nextion responde: 0x05 após cada bloco
Nextion reinicia automaticamente com a nova interface
```

Durante o upload, o display exibe uma tela azul com barra de progresso nativa. A comunicação normal (NexDisplay) fica indisponível até o Nextion reiniciar.

### Configuração

```cpp
nexOta.definirBaudOperacao(921600);  // baud atual (padrão: 9600)
nexOta.definirBaudUpload(115200);    // baud de transferência (padrão: 115200)
nexOta.definirTempoLimite(5000);     // timeout por bloco em ms

nexOta.aoProgresso([](uint8_t pct) { /* 0-100 */ });
nexOta.aoErro([](ErroNexUpload cod, const char* msg) { /* ... */ });
```

---

## Exemplos incluídos

| Exemplo | Descrição |
|---|---|
| 01_OlaMundo | Hello World — texto no display |
| 02_BotaoCallback | Callback de toque em botão |
| 03_AtualizarComponentes | set() para texto, número, cor |
| 04_NexDisplay | Configuração completa do NexDisplay |
| 05_MixCompleto | Múltiplos componentes interagindo |
| 06_DebugManager | Integração com DebugManager |
| 07_AtualizarNextionHTTP | Upload de .tft via HTTP |

---

## Compatibilidade

| Plataforma | Suporte | Observação |
|---|---|---|
| ESP32 / ESP32-S3 | ✔ Completo | Recomendado — HardwareSerial nativa |
| ESP8266 | ✔ | SoftwareSerial para segunda porta |
| AVR (Mega2560) | ✔ Limitado | Usar `NEX_BUFFER_CMD=64`, `NEX_MAX_COMPONENTES=8` |
| AVR (Uno) | ✖ | 2KB RAM insuficiente |

---

## Constantes configuráveis

Redefina **antes** do `#include` para personalizar:

```cpp
#define NEX_BUFFER_CMD        128   // buffer de comandos serial
#define NEX_MAX_COMPONENTES   32    // componentes registrados simultâneos
#define NEX_TIMEOUT_MS        100   // timeout de resposta serial
#include <NextionUI.h>
```

Para o NexAtualizador:

```cpp
#define NEX_UPLOAD_TAMANHO_BLOCO  4096   // tamanho do bloco de envio
#define NEX_UPLOAD_TIMEOUT_MS     5000   // timeout por bloco
#define NEX_UPLOAD_BAUD_PADRAO    115200 // baud de upload padrão
#include <NexAtualizador.h>
```

---

## Dependências

Nenhuma obrigatória — usa apenas Serial nativa do Arduino.

Opcional: [DebugManager](https://github.com/professorThiago/DebugManager) para logs estruturados.

---

## Licença

MIT — veja [LICENSE](LICENSE)

## Autor

**professorThiago** — [github.com/professorThiago](https://github.com/professorThiago)
