/* Week 1 / Ex2 — priorities, preemption, starvation.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "tasks.h"

#include"app_main.h"

void app_main(void)
{
    dwt_init();

    /* TODO: create the three tasks below with the priorities from params.h.
     *       Keep the stacks as small as you can still justify — ex4 will make
     *       you prove the number. */
    BaseType_t result_a = xTaskCreate(
        task_a,
        "task_a",
        128U,
        NULL,
        TASK_A_PRIO,
        NULL
    );

    BaseType_t result_b = xTaskCreate(
        task_b,
        "task_b",
        128U,
        NULL,
        TASK_B_PRIO,
        NULL
    );

    BaseType_t result_c = xTaskCreate(
        task_c,
        "task_c",
        128U,
        NULL,
        TASK_C_PRIO,
        NULL
    );

    configASSERT(result_a == pdPASS);
    configASSERT(result_b == pdPASS);
    configASSERT(result_c == pdPASS);

    vTaskStartScheduler();

    for (;;) {
    }
}

void vApplicationStackOverflowHook(
    TaskHandle_t xTask,
    char * pcTaskName
){
    __disable_irq();
    while(1){}
}

void vApplicationMallocFailedHook(void){
    __disable_irq();
    while(1){}
}
