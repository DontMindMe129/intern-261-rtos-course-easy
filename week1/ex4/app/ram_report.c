/* Week 1 / Ex4 — print the live RAM picture over USART1.
 *
 * This is your measurement instrument. It must be honest: read the values from
 * FreeRTOS, never from a constant you typed in.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"

#include <stdio.h>

#include <string.h>

#include "timers.h"

extern TaskHandle_t dynTaskHandle;
extern TaskHandle_t statTaskHandle;
extern TaskHandle_t AddTaskHandle;

/* TODO 1: a blocking UART print helper. It is called from a task, so it may
 *         block — but say in REPORT.md what would break if you called it from
 *         the stack overflow hook instead. */



extern UART_HandleTypeDef huart1;

static HAL_StatusTypeDef uart_print(const char *text)
{
    return HAL_UART_Transmit(
        &huart1,
        (uint8_t *)text,
        (uint16_t)strlen(text),
        100U
    );
}

static void print_stack(const char *name, TaskHandle_t handle)
{
    char buffer[96];

    UBaseType_t words = uxTaskGetStackHighWaterMark(handle);

    int length = snprintf(
        buffer,
        sizeof(buffer),
        "%s: stack free minimum = %lu words (%lu bytes)\r\n",
        name,
        (unsigned long)words,
        (unsigned long)(words * sizeof(StackType_t))
    );

    configASSERT(length > 0 &&
                 (size_t)length < sizeof(buffer));

    HAL_StatusTypeDef status = uart_print(buffer);
    configASSERT(status == HAL_OK);
}

void ram_report_task(void *argument)
{
    (void)argument;
    char buffer[128];

    for (;;) {
        /* TODO 2: print, every 2 seconds:
         *   - xPortGetFreeHeapSize()
         *   - xPortGetMinimumEverFreeHeapSize()
         *   - uxTaskGetStackHighWaterMark() for: dyn_task, stat_task,
         *     ram_report_task, the idle task (xTaskGetIdleTaskHandle()),
         *     and the timer task (xTimerGetTimerDaemonTaskHandle()).
         *
         * High water mark is returned in WORDS. Print both words and bytes so
         * your report cannot silently mix the two units. */

        size_t freeHeap = xPortGetFreeHeapSize();
        size_t minFreeHeap = xPortGetMinimumEverFreeHeapSize();

        int length = snprintf(
            buffer,
            sizeof(buffer),
            "Heap free: %lu bytes, minimum ever: %lu bytes\r\n",
            (unsigned long)freeHeap,
            (unsigned long)minFreeHeap
        );

        configASSERT(length > 0 &&
                     (size_t)length < sizeof(buffer));

        HAL_StatusTypeDef status = uart_print(buffer);
        configASSERT(status == HAL_OK);

        print_stack("dyn_task", dynTaskHandle);
        print_stack("stat_task", statTaskHandle);
        print_stack("ram_report", NULL);
        print_stack("idle", xTaskGetIdleTaskHandle());
        print_stack("timer", xTimerGetTimerDaemonTaskHandle());
        print_stack("add_task", AddTaskHandle);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
