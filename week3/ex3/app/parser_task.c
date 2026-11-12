/* Week 3 / Ex3 -- command parser: rx_stream -> change demo_task's
 * period/priority/suspend state at runtime.
 *
 * Commands (each terminated by '\r', max MAX_CMD_LEN bytes):
 *   P<ms>\r   -- set demo_task period to <ms>
 *   S<prio>\r -- vTaskPrioritySet(demo_task, <prio>)
 *   T<0|1>\r  -- 0 = vTaskSuspend(demo_task), 1 = vTaskResume(demo_task)
 */
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

#include "main.h"
#include "params.h"
#include "uart_rx.h"
#include "parser_task.h"
#include "uart_log.h"
#include "experiment_config.h"

TaskHandle_t demo_task_handle;
static volatile uint32_t s_demo_period_ms = 200;
volatile uint32_t g_commands_accepted;
volatile uint32_t g_commands_rejected;
volatile uint32_t g_commands_oversized;

static BaseType_t apply_command(const char *cmd, size_t len)
{
    /* TODO 1: switch on cmd[0]: 'P' -> parse decimal into s_demo_period_ms
     *         (demo_task reads this every loop, no restart needed);
     *         'S' -> parse decimal, vTaskPrioritySet(s_demo_task_handle, n);
     *         'T' -> cmd[1]=='0' ? vTaskSuspend(...) : vTaskResume(...).
     *         Ignore anything malformed -- do not crash on bad input. */
    if (len < 2U || demo_task_handle == NULL) {
        return pdFALSE;
    }

    /* Optional #sequence is measurement metadata, not part of the value. */
    size_t command_len = len;
    for (size_t i = 1U; i < len; ++i) {
        if (cmd[i] == '#') {
            command_len = i;
            if (i == 1U || i + 1U == len) {
                return pdFALSE;
            }
            uint32_t sequence = 0U;
            for (size_t j = i + 1U; j < len; ++j) {
                if (cmd[j] < '0' || cmd[j] > '9') {
                    return pdFALSE;
                }
                uint32_t digit = (uint32_t)(cmd[j] - '0');
                if (sequence > (UINT32_MAX - digit) / 10U) {
                    return pdFALSE;
                }
                sequence = sequence * 10U + digit;
            }
            break;
        }
    }

    /* Parse the entire argument as decimal; reject junk and overflow. */
    uint32_t value = 0U;
    for (size_t i = 1U; i < command_len; ++i) {
        if (cmd[i] < '0' || cmd[i] > '9') {
            return pdFALSE;
        }
        uint32_t digit = (uint32_t)(cmd[i] - '0');
        if (value > (UINT32_MAX - digit) / 10U) {
            return pdFALSE;
        }
        value = value * 10U + digit;
    }

    switch (cmd[0]) {
        case 'P': {
            /* Prevent zero-tick busy loops and tick conversion overflow. */
            uint64_t ticks = ((uint64_t)value * configTICK_RATE_HZ) / 1000U;
            if (ticks > 0U && ticks < (uint64_t)portMAX_DELAY) {
                s_demo_period_ms = value;
                return pdTRUE;
            }
            break;
        }
        case 'S':
            if (value < configMAX_PRIORITIES) {
                vTaskPrioritySet(demo_task_handle, (UBaseType_t)value);
                return pdTRUE;
            }
            break;

        case 'T':
            if (command_len == 2U && value == 0U) {
                vTaskSuspend(demo_task_handle);
                return pdTRUE;
            } else if (command_len == 2U && value == 1U) {
                vTaskResume(demo_task_handle);
                return pdTRUE;
            }
            break;

        default:
            break;
    }
    return pdFALSE;
}

static void report_stats(void)
{
    char line[EX3_TX_LINE_BYTES];
    /* Two compact lines; counters are cumulative since reset. */
    snprintf(line, sizeof(line),
             "ST rx=%lu drop=%lu uart=%lu rearm=%lu\r\n",
             (unsigned long)g_rx_bytes, (unsigned long)g_rx_stream_dropped_bytes,
             (unsigned long)g_uart_rx_errors, (unsigned long)g_uart_rearm_errors);
    uart_log_line(line);
    snprintf(line, sizeof(line),
             "ST ok=%lu bad=%lu long=%lu txdrop=%lu txerr=%lu buf=%u\r\n",
             (unsigned long)g_commands_accepted, (unsigned long)g_commands_rejected,
             (unsigned long)g_commands_oversized, (unsigned long)g_tx_dropped_lines,
             (unsigned long)g_tx_errors, (unsigned)EX3_STREAM_BYTES);
    uart_log_line(line);
}

void parser_task(void *argument)
{
    (void)argument;
    char cmd[MAX_CMD_LEN];
    size_t idx = 0;
    BaseType_t discarding = pdFALSE;

    uart_rx_start();
    uart_log_line("READY\r\n");

    for (;;) {
        uint8_t byte;

        /* TODO 2: xStreamBufferReceive(rx_stream, &byte, 1, portMAX_DELAY). */
        size_t received = xStreamBufferReceive(rx_stream, &byte, 1U, portMAX_DELAY);
        if (received != 1U) {
            continue;
        }

        /* TODO 3: if byte == '\r', call apply_command(cmd, idx), idx = 0.
         *         else if idx < MAX_CMD_LEN - 1, cmd[idx++] = byte.
         *         else (overflow): discard the whole partial command, idx = 0
         *         -- explain in REPORT.md why silently truncating instead
         *         would be worse. */
        if (byte == '\r') {
            if (discarding == pdFALSE && idx > 0U) {//nếu không lỗi và không rỗng
                cmd[idx] = '\0';
                if (idx == 1U && cmd[0] == '?') {
                    report_stats();
                } else if (apply_command(cmd, idx) == pdTRUE) {
                    ++g_commands_accepted;
                    char response[MAX_CMD_LEN + 8U];
                    /* Echo the full command so the host detects corruption
                     * of the value as well as missing sequence numbers. */
                    snprintf(response, sizeof(response), "OK %s\r\n", cmd);
                    uart_log_line(response);
                } else {
                    ++g_commands_rejected;
                    uart_log_line("ERR command\r\n");
                }
            } else if (discarding != pdFALSE) {
                uart_log_line("ERR length\r\n");
            }
            idx = 0U;
            discarding = pdFALSE;
        } 
        else if (idx == 0U && byte == '\n' && discarding == pdFALSE) {
            /* Accept terminals configured for CRLF without prefixing the
             * next command with the trailing LF. CR is still the delimiter. */
            continue;
        }
        else if (discarding == pdTRUE) {
            /* Ignore the entire oversized command, including its tail. */
            continue;
        } 
        else if (idx < sizeof(cmd) - 1U) {
            cmd[idx++] = (char)byte;
        } 
        else {
            ++g_commands_oversized;
            idx = 0U;
            discarding = pdTRUE;
        }
    }
}

void demo_task(void *argument)
{
    (void)argument;

    for (;;) {
        /* TODO 4: toggle a GPIO pin here. */
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
        vTaskDelay(pdMS_TO_TICKS(s_demo_period_ms));
    }
}
