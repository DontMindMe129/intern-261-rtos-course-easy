/* Week 2 / Ex4 — event group & task notification.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

#include "main.h"
#include "params.h"
#include "sync_tasks.h"
#include "multi_waiters.h"

void app_main(void)
{
#if EX4_TEST_CASE == EX4_MULTI_WAITERS
    multi_waiters_init();
#else
    sync_init();

    /* TODO 1: xTaskCreate sync_task first, store its handle into
     *         sync_task_handle — producer_0 needs it when
     *         PRODUCER0_USE_NOTIFY is set. */
    BaseType_t result = xTaskCreate(sync_task, "sync", 256U, NULL, 2U, &sync_task_handle);
    configASSERT(result == pdPASS);

    /* TODO 2: xTaskCreate EX4_PRODUCER_COUNT instances of producer_task, each
     *         given its own index (0..EVENT_BIT_COUNT-1) as the task
     *         parameter. */
    configASSERT(EX4_PRODUCER_COUNT > 0 && EX4_PRODUCER_COUNT <= 24);
    for (uint32_t idx = 0; idx < EX4_PRODUCER_COUNT; idx++) {
        char name[12];
        (void)snprintf(name, sizeof(name), "producer%lu", (unsigned long)idx);

        result = xTaskCreate(producer_task, name, 256U, (void *)(uintptr_t)idx, 1U, NULL);
        configASSERT(result == pdPASS);
    }

#endif

    vTaskStartScheduler();
    Error_Handler(); /* Scheduler chỉ trả về nếu khởi động thất bại. */

    for (;;) {
    }
}
