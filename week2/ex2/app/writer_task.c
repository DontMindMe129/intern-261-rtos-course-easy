/* Week 2 / Ex2 — WRITER_COUNT identical writer tasks. Each prints an MSG_LEN
 * byte string with its own id + sequence number, every WRITER_PERIOD_MS.
 */
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "params.h"
#include "uart_guard.h"

/* TODO 1: one writer_task(void *argument) taking the writer index via the
 *         task parameter (argument), NOT WRITER_COUNT copy-pasted bodies.
 *         Format a MSG_LEN-byte string containing the writer id and a
 *         per-writer sequence number, then uart_guard_send() it, then
 *         vTaskDelay(pdMS_TO_TICKS(WRITER_PERIOD_MS)). */
void writer_task(void *argument)
{
    uint32_t id = (uint32_t)(uintptr_t)argument;
    uint32_t seq = 0;
    (void)id;

    configASSERT(id < WRITER_COUNT);
    configASSERT(WRITER_COUNT <= 26); /* A..Z phân biệt nguồn ký tự. */
    configASSERT(pdMS_TO_TICKS(WRITER_PERIOD_MS) > 0);

    for (;;) {
        /* TODO 2 */
        char msg[MSG_LEN + 1]; /* Thêm chỗ cho '\0'; chỉ truyền MSG_LEN byte. */

        int header_len = snprintf(msg, sizeof(msg), "[T%lu #%010lu] ",
                                  (unsigned long)id, (unsigned long)seq++);

        if (header_len < 0 || (size_t)header_len >= MSG_LEN - 2U) {

            Error_Handler();

        }

        memset(msg + header_len, (int)('A' + id), MSG_LEN - 2U - (size_t)header_len);

        msg[MSG_LEN - 2U] = '\r';

        msg[MSG_LEN - 1U] = '\n';

        msg[MSG_LEN] = '\0';

        uart_guard_send(msg, MSG_LEN);


        vTaskDelay(pdMS_TO_TICKS(WRITER_PERIOD_MS));
        
    }
}
