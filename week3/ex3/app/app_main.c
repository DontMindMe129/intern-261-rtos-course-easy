/* Week 3 / Ex3 -- UART RX -> stream buffer -> parser.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "uart_rx.h"
#include "parser_task.h"
#include "uart_log.h"

void app_main(void)
{
    uart_rx_init();
    uart_log_init();

    /* TODO: xTaskCreate parser_task and demo_task. parser_task must run at a
     *       priority high enough that it drains rx_stream faster than bytes
     *       arrive at RX_BAUD -- justify the number in REPORT.md with the
     *       byte time you computed. */
    BaseType_t result = xTaskCreate(demo_task, "demo", 256U, NULL, 1U,
                                   &demo_task_handle);
    configASSERT(result == pdPASS);

    /* The handle is already valid even if parser runs before demo. */
    /* TX polling stays below the initial demo priority, so logging does
     * not preempt demo when it wakes. The parser remains highest. */
    result = xTaskCreate(uart_log_task, "uart_tx", 384U, NULL, 0U, NULL);
    configASSERT(result == pdPASS);

    result = xTaskCreate(parser_task, "parser", 512U, NULL, 3U, NULL);
    configASSERT(result == pdPASS);

    vTaskStartScheduler();

    Error_Handler();

    for (;;) {
    }
}
