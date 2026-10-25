#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    __disable_irq();
    for (;;) { }
}

void vApplicationMallocFailedHook(void)
{
    __disable_irq();
    for (;;) { }
}
