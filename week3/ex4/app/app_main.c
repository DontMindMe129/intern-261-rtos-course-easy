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

void app_main(void)
{
    /* Kernel initializes DWT through portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
     * inside vTaskStartScheduler(), before starting the first task. */
    deadlock_demo_init();
    /* First experiment: no supervisor or IWDG, so the original deadlock
     * remains available for inspection instead of resetting the board. */

    /* TODO: xTaskCreate task_alpha, task_beta, supervisor_task,
     *       runtime_stats_task. Run deadlock_demo exactly as given first
     *       (step 2 of README) before touching task_alpha/task_beta. */
    BaseType_t result = xTaskCreate(task_alpha, "alpha", 256U, NULL, 2U, NULL);
    configASSERT(result == pdPASS);
    result = xTaskCreate(task_beta, "beta", 256U, NULL, 2U, NULL);
    configASSERT(result == pdPASS);
    result = xTaskCreate(runtime_stats_task, "stats", 768U, NULL, 1U, NULL);
    configASSERT(result == pdPASS);

    vTaskStartScheduler();

    Error_Handler();

    for (;;) {
    }
}
