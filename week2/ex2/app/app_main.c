/* Week 2 / Ex2 — race condition & mutex.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

#include "main.h"
#include "params.h"
#include "uart_guard.h"
#ifdef EX2_MEASUREMENT_ENABLED
#include "measurement.h"
#endif

void writer_task(void *argument);

void app_main(void)
{
#ifdef EX2_MEASUREMENT_ENABLED
    measurement_init();
    /* heap_4 reports 0 until its first allocation. Prime it before sampling. */
    void *heap_probe = pvPortMalloc(1U);
    configASSERT(heap_probe != NULL);
    vPortFree(heap_probe);
    size_t heap_before_guard = xPortGetFreeHeapSize();
#endif
    uart_guard_init();
#ifdef EX2_MEASUREMENT_ENABLED
    measurement_guard_heap_bytes = heap_before_guard - xPortGetFreeHeapSize();
#endif

    /* TODO: xTaskCreate WRITER_COUNT instances of writer_task, each given its
     *       own index (0..WRITER_COUNT-1) as the task parameter.
     *       Do not create WRITER_COUNT separately-named functions. */

    for (uint32_t id = 0; id < WRITER_COUNT; id++) {

        char name[12];

        (void)snprintf(name, sizeof(name), "writer%lu", (unsigned long)id);

        BaseType_t result = xTaskCreate(writer_task, name, 256U,
                                       (void *)(uintptr_t)id, 1U, NULL);
                                       
        configASSERT(result == pdPASS);
    }

#ifdef EX2_MEASUREMENT_ENABLED
    measurement_heap_free_after_writers = xPortGetFreeHeapSize();
#endif
    vTaskStartScheduler();

    Error_Handler();

    for (;;) {
    }
}
