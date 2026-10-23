/* Week 2 / Ex3 — priority inversion.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "tasks_lmh.h"

void app_main(void)
{
    resource_init();

    /* TODO: xTaskCreate task_low, task_mid, task_high with three DISTINCT
     *       priorities, low < mid < high. Justify the exact numbers in
     *       REPORT.md — they must leave room for configMAX_PRIORITIES and not
     *       collide with any other task in the system. */

    BaseType_t result = xTaskCreate(task_low, "low", 256U, NULL, 1U, NULL);
    configASSERT(result == pdPASS);

    result = xTaskCreate(task_mid, "mid", 256U, NULL, 2U, NULL);
    configASSERT(result == pdPASS);

    result = xTaskCreate(task_high, "high", 256U, NULL, 3U, NULL);
    configASSERT(result == pdPASS);

    vTaskStartScheduler();

    Error_Handler(); /* Scheduler chỉ trả về nếu khởi động thất bại. */

    for (;;) {
    }
}
