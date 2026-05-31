/**
 * @file NextionUI.h
 * @brief Ponto único de inclusão da biblioteca NextionUI.
 *
 * @details
 * Inclua apenas este arquivo no seu sketch:
 *
 * @code
 * #include <NextionUI.h>
 * @endcode
 *
 * ## Visão geral da biblioteca
 *
 * NextionUI é uma biblioteca moderna para displays Nextion no ESP32,
 * desenvolvida como evolução da ITEADLIB_Arduino_Nextion com foco em:
 *
 * - **API em português** — nomes de métodos e parâmetros em PT-BR
 * - **Zero alocações dinâmicas** — buffer fixo com `snprintf`, sem `String`
 * - **Fluent interface** — encadeamento de métodos
 * - **Ponto de entrada único** — classe `NexDisplay` agrupa tudo
 * - **Callbacks modernos** — suporte a lambdas
 * - **Integração com DebugManager** — inclua DebugManager antes deste arquivo
 * - **Doxygen completo** — toda a API documentada em português
 *
 * ## Início rápido
 *
 * ### 1. Inicializar o display
 *
 * @code
 * #include <NextionUI.h>
 *
 * NexDisplay display;
 * NexBotao   btnLigar (0, 2, "btn_ligar");
 * NexTexto   tStatus  (0, 3, "t_status");
 *
 * void setup() {
 *     // ESP32: RX=GPIO16, TX=GPIO17, Serial2
 *     display.begin(Serial2, 115200, 16, 17);
 *
 *     // Registrar componentes para receber toque
 *     display.escutar(btnLigar);
 *
 *     // Registrar callback
 *     btnLigar.aoSoltar([]() {
 *         tStatus.texto("LIGADO");
 *         tStatus.corFundo(RGB565(0, 150, 0));
 *     });
 * }
 *
 * void loop() {
 *     display.atualizar();   // processa eventos de toque
 * }
 * @endcode
 *
 * ### 2. Atualizar componentes
 *
 * @code
 * // Texto simples
 * tStatus.texto("Olá, mundo!");
 *
 * // Texto formatado (como printf)
 * tTemp.setf("txt", "%d°C", temperatura);
 *
 * // Atributo genérico
 * btn.set("bco", RGB565(200, 0, 0));   // cor de fundo vermelha
 * btn.set("val", 42);                  // valor numérico
 *
 * // Encadeamento
 * btn.texto("ATIVO")
 *    .corFundo(RGB565(0, 120, 200))
 *    .corTexto(COR_BRANCO)
 *    .visivel(true);
 * @endcode
 *
 * ### 3. Navegar entre páginas
 *
 * @code
 * display.irPara("p2_ac");    // por nome
 * display.irPara(2);           // por índice
 *
 * display.aoMudarPagina([](uint8_t pg) {
 *     if (pg == 5) iniciarScan();
 * });
 * @endcode
 *
 * ## Configuração
 *
 * Redefina as macros abaixo **antes** de incluir este arquivo:
 *
 * @code
 * #define NEX_BUFFER_CMD       256   // buffer de comando (padrão: 128)
 * #define NEX_MAX_COMPONENTES   64   // lista de escuta (padrão: 32)
 * #define NEX_TIMEOUT_MS       200   // timeout serial (padrão: 100)
 * #define NEX_DEBUG_SERIAL  Serial   // ativar debug (padrão: desativado)
 * #include <NextionUI.h>
 * @endcode
 *
 * ## Componentes disponíveis
 *
 * | Classe                 | Tipo Nextion      | Descrição                    |
 * |------------------------|-------------------|------------------------------|
 * | `NexBotao`             | Button            | Botão com texto              |
 * | `NexTexto`             | Text              | Rótulo de texto              |
 * | `NexNumero`            | Number            | Valor numérico               |
 * | `NexSlider`            | Slider            | Controle deslizante          |
 * | `NexBarraProgresso`    | ProgressBar       | Barra de progresso           |
 * | `NexGrafico`           | Waveform          | Gráfico de forma de onda     |
 * | `NexGauge`             | Gauge             | Mostrador circular           |
 * | `NexCheckbox`          | Checkbox          | Caixa de seleção             |
 * | `NexRadio`             | Radio             | Botão de rádio               |
 * | `NexBotaoDuplo`        | DualStateButton   | Botão toggle ON/OFF          |
 * | `NexVariavel`          | Variable          | Variável interna             |
 * | `NexTimer`             | Timer             | Timer periódico              |
 * | `NexTextoDeslizante`   | Scrolltext        | Texto com rolagem            |
 * | `NexImagem`            | Picture           | Imagem estática              |
 * | `NexCrop`              | Crop              | Imagem recortada             |
 * | `NexHotspot`           | Hotspot           | Área de toque invisível      |
 * | `NexPagina`            | Page              | Página navegável             |
 * | `NexRtc`               | RTC               | Relógio em tempo real        |
 * | `NexGpio`              | GPIO              | Pinos GPIO do Nextion        |
 *
 * ## Cores em RGB565
 *
 * @code
 * RGB565(r, g, b)    // converte RGB (0-255) para RGB565
 *
 * // Constantes prontas:
 * COR_PRETO    COR_BRANCO   COR_VERMELHO
 * COR_VERDE    COR_AZUL     COR_AMARELO
 * COR_CIANO    COR_MAGENTA  COR_CINZA
 * @endcode
 *
 * @author  professorThiago (https://github.com/professorThiago)
 * @version 1.0.0
 * @date    2025
 * @license MIT
 *
 * @par Licença MIT
 * Copyright (c) 2025 professorThiago\n
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:\n
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.\n
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
 */

#ifndef __NEXTIONUI_H__
#define __NEXTIONUI_H__

// Inclui tudo na ordem correta
#include "NexConfig.h"
#include "NexHardware.h"
#include "NexObjeto.h"
#include "NexToque.h"
#include "NexComponentes.h"
#include "NexDisplay.h"

#endif /* __NEXTIONUI_H__ */
