/* Week 1 / Ex3 — entry point. */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"

#include "dwt.h"
#include "app_main.h"

void periodic_task(void *argument);

void app_main(void)
{
    dwt_init();

    /* TODO: create periodic_task. Its priority must be the highest in the system
     *       — justify why in REPORT.md, and say what the measurement would look
     *       like if it were not. */

    BaseType_t result = xTaskCreate(
        periodic_task,
        "periodic_task",
        128U,
        NULL,
        1,
        NULL
    );

    configASSERT(result == pdPASS);

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