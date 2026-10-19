/* Week 1 / Ex2 — the three competing tasks.
 *
 * Each task: raise its pin, burn exactly TASK_x_WORK_US of CPU, lower its pin,
 * and loop again WITHOUT blocking. The missing block is the point of the
 * exercise, not a bug to fix in step 2.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "tasks.h"

static void capture_runtime_once(void);

/* TODO: one helper that takes a port/pin and a work time, then three thin
 *       wrappers. Do not copy-paste the body three times. */
void helper(GPIO_TypeDef *port, uint16_t pin, uint32_t work_us){
    HAL_GPIO_WritePin(port, pin, 1);
    dwt_busy_wait_us((uint32_t)work_us);
    HAL_GPIO_WritePin(port, pin, 0);
}

void task_a(void *argument)
{
    (void)argument;
    for (;;) {
        /* TODO */
        helper(GPIOA, (1U << TASK_A_PIN), TASK_A_WORK_US);

        capture_runtime_once();

        vTaskDelay(1);
    }
}

void task_b(void *argument)
{
    (void)argument;
    for (;;) {
        /* TODO */
        helper(GPIOA, (1U << TASK_B_PIN), TASK_B_WORK_US);
        vTaskDelay(1);
    }
}

void task_c(void *argument)
{
    (void)argument;
    for (;;) {
        /* TODO */
        helper(GPIOA, (1U << TASK_C_PIN), TASK_C_WORK_US);
        vTaskDelay(1);
    }
}

/* A, B, C và Idle: tổng cộng 4 task. */
TaskStatus_t runtimeTasks[4];
configRUN_TIME_COUNTER_TYPE runtimeTotal;
volatile UBaseType_t runtimeTaskCount = 0;

static void capture_runtime_once(void)
{
    static BaseType_t captured = pdFALSE;

    if ((captured == pdFALSE) &&
        (DWT->CYCCNT >= configCPU_CLOCK_HZ * 10U)) {

        runtimeTaskCount = uxTaskGetSystemState(
            runtimeTasks,
            4U,
            &runtimeTotal
        );

        captured = pdTRUE;
    }
}