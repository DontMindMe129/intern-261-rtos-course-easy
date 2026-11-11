/* Week 3 / Ex1 -- ISR -> task wakeup, NVIC priority.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "exti_button.h"
#include "experiment_config.h"

volatile uint32_t g_assert_failed = 0U;

void app_main(void)
{
    exti_button_init();

    /* TODO: xTaskCreate ack_task. Keep its priority high enough that it is
     *       the thing that actually determines your measured latency, not
     *       scheduling jitter from something else. Justify the number in
     *       REPORT.md. */
    BaseType_t result = xTaskCreate(ack_task, "ack", EX1_ACK_TASK_STACK_WORDS,
                                   NULL, EX1_ACK_TASK_PRIORITY, NULL);
    configASSERT(result == pdPASS);

    vTaskStartScheduler();

    /* The scheduler only returns if it could not start. */
    Error_Handler();

    for (;;) {
    }
}
