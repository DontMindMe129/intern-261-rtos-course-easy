/* Week 1 / Ex4 — FreeRTOS hooks.
 *
 * Both hooks must be observable from outside the chip. A breakpoint is not a
 * deliverable: you need a UART line or a distinct LED pattern you can photograph.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"

static void fault_uart_print(const char *text)
{
    while (*text != '\0') {
        uint32_t timeout = 100000U;

        while ((USART1->SR & USART_SR_TXE) == 0U) {
            if (--timeout == 0U) {
                return;
            }
        }

        USART1->DR = (uint8_t)*text;
        text++;
    }

    uint32_t timeout = 100000U;

    while ((USART1->SR & USART_SR_TC) == 0U) {
        if (--timeout == 0U) {
            return;
        }
    }
}



void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    /* TODO 1: report xTask / pcTaskName out of the chip, then stop.
     *
     * Careful: the stack of the offending task is already corrupt. Think about
     * what you are allowed to call here, and write the reasoning in REPORT.md.
     * Hint: this hook runs from the scheduler's context, not the task's.
     */

    (void)xTask;
    (void)pcTaskName;

    __disable_irq();

    fault_uart_print("\r\nFAULT: stack overflow\r\n");
    
    for (;;) {
        
    }
}

void vApplicationMallocFailedHook(void)
{
    /* TODO 2: report, then stop. */

    __disable_irq();

    fault_uart_print("\r\nFAULT: malloc failed\r\n");

    for (;;) {
    }
}

/* configSUPPORT_STATIC_ALLOCATION = 1 makes these two callbacks mandatory.
 * Provide the memory for the idle task and the timer task here. */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    /* TODO 3: static buffers. Note the size is in WORDS, not bytes —
     *         getting this wrong is a silent overflow. */
    static StaticTask_t idleTCB;
    static StackType_t idleStack[configMINIMAL_STACK_SIZE];

    *ppxIdleTaskTCBBuffer = &idleTCB;
    *ppxIdleTaskStackBuffer = idleStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
    /* TODO 4: same, for the software timer task. */
    static StaticTask_t timerTCB;
    static StackType_t timerStack[configTIMER_TASK_STACK_DEPTH];

    *ppxTimerTaskTCBBuffer = &timerTCB;
    *ppxTimerTaskStackBuffer = timerStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
