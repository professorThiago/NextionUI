/**
 * @file NexHardware.h
 * @brief Camada de hardware da biblioteca NextionUI.
 *
 * @details
 * Gerencia a comunicação serial com o display Nextion. Todos os envios
 * e recepções passam por esta camada.
 *
 * ### Por que buffer fixo em vez de String?
 *
 * A classe `String` do Arduino aloca memória no heap dinamicamente.
 * Em sistemas embarcados, chamadas repetidas de alocação/liberação
 * fragmentam o heap, causando comportamento imprevisível ao longo do
 * tempo (crashes, dados corrompidos). Esta biblioteca usa um buffer
 * estático `char[]` com `snprintf()` para eliminar completamente
 * esse problema.
 *
 * ### Protocolo Nextion
 *
 * Todo comando é terminado com três bytes `0xFF`:
 * ```
 * "t0.txt=\"Olá\""  0xFF  0xFF  0xFF
 * ```
 *
 * Todo retorno de dados também termina com três bytes `0xFF`.
 *
 * @author  professorThiago
 * @version 1.0.0
 * @license MIT
 */

#ifndef __NEXHARDWARE_H__
#define __NEXHARDWARE_H__

#include <Arduino.h>
#include "NexConfig.h"

// ---------------------------------------------------------------------------
// Códigos de retorno do protocolo Nextion
// ---------------------------------------------------------------------------

/** @brief Comando executado com sucesso. */
#define NEX_RET_CMD_OK              0x01

/** @brief Cabeçalho de evento de toque. */
#define NEX_RET_TOQUE               0x65

/** @brief Cabeçalho de mudança de página. */
#define NEX_RET_MUDANCA_PAGINA      0x66

/** @brief Cabeçalho de retorno de string. */
#define NEX_RET_STRING              0x70

/** @brief Cabeçalho de retorno de número. */
#define NEX_RET_NUMERO              0x71

/** @brief Nextion inicializou (enviado no boot). */
#define NEX_RET_INICIALIZADO        0x88

// ---------------------------------------------------------------------------
// API de hardware (funções globais — usadas internamente pelos componentes)
// ---------------------------------------------------------------------------

/**
 * @brief Inicializa a comunicação serial com o display Nextion.
 *
 * @details
 * Configura a porta serial, envia comandos iniciais (`bkcmd=0` para
 * suprimir retornos desnecessários) e aguarda o display estabilizar.
 *
 * @param baudrate   Velocidade da comunicação. Padrão: 9600.
 *                   Recomendado: 115200 para melhor desempenho.
 * @param pinoRX     Pino RX do ESP32 (recebe dados do Nextion TX).
 *                   Use -1 para o padrão da porta serial escolhida.
 * @param pinoTX     Pino TX do ESP32 (envia dados para o Nextion RX).
 *                   Use -1 para o padrão da porta serial escolhida.
 * @param serial     Ponteiro para a porta serial a usar.
 *                   Padrão: `&Serial1`.
 *
 * @return true  se inicializado com sucesso.
 * @return false se falhou (porta serial inválida).
 *
 * @par Exemplo (ESP32 com pinos customizados)
 * @code
 * // GPIO16 = RX do ESP32 (liga no TX do Nextion)
 * // GPIO17 = TX do ESP32 (liga no RX do Nextion)
 * nexIniciar(115200, 16, 17, &Serial2);
 * @endcode
 *
 * @par Exemplo (padrão Serial1)
 * @code
 * nexIniciar(9600);
 * @endcode
 */
bool nexIniciar(uint32_t baudrate = 9600,
                int8_t   pinoRX   = -1,
                int8_t   pinoTX   = -1,
                HardwareSerial *serial = &Serial1);

/**
 * @brief Encerra a comunicação serial com o Nextion.
 */
void nexEncerrar();

/**
 * @brief Retorna o ponteiro para a porta serial ativa do Nextion.
 *
 * Útil quando é necessário enviar comandos brutos diretamente.
 *
 * @return Ponteiro para HardwareSerial.
 */
HardwareSerial* nexSerial();

/**
 * @brief Envia um comando ao Nextion seguido do terminador 0xFF 0xFF 0xFF.
 *
 * @details
 * Descarta qualquer dado pendente no buffer de entrada antes de enviar,
 * garantindo que a resposta seguinte pertença a este comando.
 *
 * @param cmd  String de comando terminada em `\0`.
 *
 * @par Exemplo
 * @code
 * nexEnviarCmd("page p1_dash");
 * nexEnviarCmd("t0.txt=\"Olá\"");
 * @endcode
 */
void nexEnviarCmd(const char* cmd);

/**
 * @brief Envia um comando formatado (estilo printf) ao Nextion.
 *
 * @details
 * Equivalente a `snprintf` + `nexEnviarCmd`. Usa o buffer interno fixo
 * de `NEX_BUFFER_CMD` bytes — sem alocação no heap.
 *
 * @param fmt   String de formato (igual a `printf`).
 * @param ...   Argumentos de formatação.
 *
 * @par Exemplo
 * @code
 * nexEnviarCmdF("t0.txt=\"%d°C\"", temperatura);
 * nexEnviarCmdF("%s.bco=%lu", nomeComponente, corFundo);
 * nexEnviarCmdF("page %d", indicePagina);
 * @endcode
 */
void nexEnviarCmdF(const char* fmt, ...);

/**
 * @brief Aguarda e valida a confirmação de execução de comando.
 *
 * @param timeout  Tempo máximo de espera em milissegundos.
 *
 * @return true  se recebeu `0x01 0xFF 0xFF 0xFF` (comando OK).
 * @return false se timeout ou código de erro.
 *
 * @note Só tem efeito quando `NEX_AGUARDAR_CONFIRMACAO = 1`.
 *       Com `= 0` (padrão) retorna sempre `true` imediatamente.
 */
bool nexAguardarOk(uint32_t timeout = NEX_TIMEOUT_MS);

/**
 * @brief Lê um número (uint32_t) enviado pelo Nextion.
 *
 * @details
 * Formato esperado do Nextion: `0x71 <b0> <b1> <b2> <b3> 0xFF 0xFF 0xFF`
 * (little-endian, 4 bytes).
 *
 * @param[out] numero  Ponteiro onde o número lido será armazenado.
 * @param      timeout Timeout em ms.
 *
 * @return true  se lido com sucesso.
 * @return false se timeout ou formato inválido.
 *
 * @par Exemplo
 * @code
 * uint32_t val;
 * nexEnviarCmd("get n0.val");
 * if (nexLerNumero(&val)) {
 *     Serial.println(val);
 * }
 * @endcode
 */
bool nexLerNumero(uint32_t *numero, uint32_t timeout = NEX_TIMEOUT_MS);

/**
 * @brief Lê uma string enviada pelo Nextion.
 *
 * @details
 * Formato esperado: `0x70 <chars...> 0xFF 0xFF 0xFF`.
 *
 * @param      buffer   Buffer destino.
 * @param      tamanho  Tamanho máximo do buffer (incluindo `\0`).
 * @param      timeout  Timeout em ms.
 *
 * @return Comprimento da string lida (sem o `\0`).
 *
 * @par Exemplo
 * @code
 * char texto[32];
 * nexEnviarCmd("get t0.txt");
 * uint16_t len = nexLerString(texto, sizeof(texto));
 * Serial.println(texto);
 * @endcode
 */
uint16_t nexLerString(char *buffer, uint16_t tamanho,
                       uint32_t timeout = NEX_TIMEOUT_MS);

#endif /* __NEXHARDWARE_H__ */
