/**
 * @file NexConfig.h
 * @brief Configuração global da biblioteca NextionUI.
 *
 * @details
 * Este arquivo centraliza todas as opções configuráveis da biblioteca.
 * Redefina qualquer macro **antes** de incluir `NextionUI.h` para
 * personalizar o comportamento sem alterar o código-fonte da biblioteca.
 *
 * ### Debug
 *
 * Por padrão o debug está **desativado**. Para ativá-lo:
 *
 * @code
 * // Opção 1 — usar DebugManager (recomendado se já estiver no projeto)
 * #include <DebugManager.h>
 * #include <NextionUI.h>
 *
 * // Opção 2 — ativar debug próprio via Serial
 * #define NEX_DEBUG_SERIAL Serial
 * #include <NextionUI.h>
 * @endcode
 *
 * ### Ajustar tamanho do buffer de comando
 *
 * @code
 * #define NEX_BUFFER_CMD 256    // padrão: 128 bytes
 * #include <NextionUI.h>
 * @endcode
 *
 * ### Desativar espera de confirmação de comando
 *
 * @code
 * #define NEX_AGUARDAR_CONFIRMACAO 0   // padrão: 0 (desativado = mais rápido)
 * #include <NextionUI.h>
 * @endcode
 *
 * @author  professorThiago
 * @version 1.0.0
 * @license MIT
 */

#ifndef __NEXCONFIG_H__
#define __NEXCONFIG_H__

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Tamanho do buffer de comando (bytes)
// Aumentar se usar nomes de componentes muito longos ou textos grandes.
// ---------------------------------------------------------------------------
#ifndef NEX_BUFFER_CMD
  #define NEX_BUFFER_CMD  128
#endif

// ---------------------------------------------------------------------------
// Aguardar confirmação (0x01 0xFF 0xFF 0xFF) após cada comando enviado.
// 0 = não aguarda (mais rápido, menos seguro)
// 1 = aguarda (mais lento, detecta erros de comando)
// ---------------------------------------------------------------------------
#ifndef NEX_AGUARDAR_CONFIRMACAO
  #define NEX_AGUARDAR_CONFIRMACAO  0
#endif

// ---------------------------------------------------------------------------
// Timeout padrão para leitura serial (ms)
// ---------------------------------------------------------------------------
#ifndef NEX_TIMEOUT_MS
  #define NEX_TIMEOUT_MS  100
#endif

// ---------------------------------------------------------------------------
// Número máximo de componentes na lista de escuta de toque
// ---------------------------------------------------------------------------
#ifndef NEX_MAX_COMPONENTES
  #define NEX_MAX_COMPONENTES  32
#endif

// ---------------------------------------------------------------------------
// Macros de debug
//
// Prioridade:
//   1. DebugManager — se o header já tiver sido incluído antes deste arquivo
//   2. NEX_DEBUG_SERIAL — se o usuário definiu uma porta serial
//   3. Silencioso — padrão (sem output)
// ---------------------------------------------------------------------------

#if defined(DEBUG_MANAGER_H)
  // Integração com DebugManager (professorThiago/DebugManager)
  #define _NEX_DBG(msg)    debugVerbose("[Nextion] " + String(msg))
  #define _NEX_DBG_ERR(msg) debugErro  ("[Nextion] " + String(msg))
  #define _NEX_DBG_INFO(msg) debugInfo ("[Nextion] " + String(msg))

#elif defined(NEX_DEBUG_SERIAL)
  // Debug via Serial definido pelo usuário
  #define _NEX_DBG(msg)     do { NEX_DEBUG_SERIAL.print("[NEX] "); NEX_DEBUG_SERIAL.println(msg); } while(0)
  #define _NEX_DBG_ERR(msg) do { NEX_DEBUG_SERIAL.print("[NEX-ERR] "); NEX_DEBUG_SERIAL.println(msg); } while(0)
  #define _NEX_DBG_INFO(msg)do { NEX_DEBUG_SERIAL.print("[NEX-INF] "); NEX_DEBUG_SERIAL.println(msg); } while(0)

#else
  // Silencioso (padrão)
  #define _NEX_DBG(msg)      do {} while(0)
  #define _NEX_DBG_ERR(msg)  do {} while(0)
  #define _NEX_DBG_INFO(msg) do {} while(0)
#endif

#endif /* __NEXCONFIG_H__ */
