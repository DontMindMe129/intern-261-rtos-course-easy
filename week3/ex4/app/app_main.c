/* Week 3 / Ex4 -- CPU load, deadlock, watchdog.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "runtime_stats.h"
#include "deadlock_demo.h"
#include "supervisor.h"
#include <string.h>

extern UART_HandleTypeDef huart1;

void app_main(void)
{
    /* Kernel initializes DWT through portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
     * inside vTaskStartScheduler(), before starting the first task. */
    deadlock_demo_init();
    supervisor_init();
    const char *boot = __HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)
        ? "\r\nBOOT normal reset=IWDG\r\n"
        : "\r\nBOOT normal reset=other\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t *)boot, (uint16_t)strlen(boot), 100U);
    __HAL_RCC_CLEAR_RESET_FLAGS();
    /* Normal operation: matching mutex orders, three check-ins, IWDG enabled. */

    /* TODO: xTaskCreate task_alpha, task_beta, supervisor_task,
     *       runtime_stats_task. Run deadlock_demo exactly as given first
     *       (step 2 of README) before touching task_alpha/task_beta. */
    BaseType_t result = xTaskCreate(task_alpha, "alpha", 256U, NULL, 2U, NULL);
    configASSERT(result == pdPASS);
    result = xTaskCreate(task_beta, "beta", 256U, NULL, 2U, NULL);
    configASSERT(result == pdPASS);
    result = xTaskCreate(runtime_stats_task, "stats", 768U, NULL, 1U, NULL);
    configASSERT(result == pdPASS);
    result = xTaskCreate(supervisor_task, "supervisor", 256U, NULL, 3U, NULL);
    configASSERT(result == pdPASS);

    vTaskStartScheduler();

    Error_Handler();

    for (;;) {
    }
}
