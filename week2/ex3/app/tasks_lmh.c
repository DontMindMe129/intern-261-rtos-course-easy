/* Week 2 / Ex3 — priority inversion vs priority inheritance.
 *
 * task_low  (lowest prio)  holds the shared resource for LOW_HOLD_MS.
 * task_mid  (middle prio)  never touches the resource; pure CPU-bound work,
 *                          its only job is to be ready and preemptible so it
 *                          can steal CPU from task_low while task_low holds
 *                          the resource task_high is waiting for.
 * task_high (highest prio) needs the resource every HIGH_PERIOD_MS and is
 *                          the one whose block time we measure.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "main.h"
#include "params.h"
#include "tasks_lmh.h"

static SemaphoreHandle_t s_resource;

#define WORK_CALIBRATION_LOOPS 10000U
static uint32_t s_calibration_cycles;

/* Fixed work: preemption pauses the loop instead of consuming its budget. */
static void __attribute__((noinline)) spin_iterations(uint32_t iterations)
{
    for (volatile uint32_t i = 0; i < iterations; i++) {
        __NOP();
    }
}

static void busy_work_ms(uint32_t duration_ms)
{
    configASSERT(s_calibration_cycles != 0U);

    uint64_t iterations = (uint64_t)(SystemCoreClock / 1000U) * duration_ms * WORK_CALIBRATION_LOOPS / s_calibration_cycles;

    configASSERT(iterations > 0U && iterations <= UINT32_MAX);
    
    spin_iterations((uint32_t)iterations);
}

void resource_init(void)
{
    /* Calibrate approximate CPU work once, before starting the scheduler. */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    uint32_t start = DWT->CYCCNT;
    spin_iterations(WORK_CALIBRATION_LOOPS);
    s_calibration_cycles = DWT->CYCCNT - start;
    configASSERT(s_calibration_cycles != 0U);

#if RESOURCE_MODE == 0
    /* TODO 1: xSemaphoreCreateBinary(), then give it once so it starts
     *         "available" (binary semaphores start empty). */
    s_resource = xSemaphoreCreateBinary();
    configASSERT(s_resource != NULL);

    BaseType_t result = xSemaphoreGive(s_resource);
    configASSERT(result == pdTRUE);
#else
    /* TODO 2: xSemaphoreCreateMutex(). */
    s_resource = xSemaphoreCreateMutex();
    configASSERT(s_resource != NULL);
#endif
}

void task_low(void *argument)
{
    (void)argument;
    const TickType_t period = pdMS_TO_TICKS(2U * HIGH_PERIOD_MS);
    configASSERT(period > 0U);
    TickType_t last_wake = xTaskGetTickCount();

    for (;;) {
        /* TODO 3: take s_resource (portMAX_DELAY), toggle a GPIO marking
         *         "resource held", busy-wait LOW_HOLD_MS using the ex2 DWT
         *         helper or a HAL tick-based wait, lower the GPIO, give
         *         s_resource back. Then yield/delay briefly before looping —
         *         this task must not monopolize the CPU outside the hold. */
        if (xSemaphoreTake(s_resource, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
        busy_work_ms(LOW_HOLD_MS);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);

        BaseType_t result = xSemaphoreGive(s_resource);
        configASSERT(result == pdTRUE);

        /* Repeat the experiment every 100 ms, matching Mid's period. */
        vTaskDelayUntil(&last_wake, period);
    }
}

void task_mid(void *argument)
{
    (void)argument;
    const TickType_t period = pdMS_TO_TICKS(2U * HIGH_PERIOD_MS);
    configASSERT(period > pdMS_TO_TICKS(MID_WORK_MS));

    /* Let Low acquire the resource before Mid first becomes Ready. */
    vTaskDelay(pdMS_TO_TICKS(2U));
    TickType_t last_wake = xTaskGetTickCount();

    for (;;) {
        /* TODO 4: toggle a GPIO, busy-wait MID_WORK_MS, lower the GPIO, then
         *         vTaskDelay a short period so it keeps becoming Ready
         *         periodically rather than running once and blocking
         *         forever. This task never touches s_resource. */
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
        busy_work_ms(MID_WORK_MS);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

        vTaskDelayUntil(&last_wake, period);
    }
}

void task_high(void *argument)
{
    (void)argument;
    const TickType_t period = pdMS_TO_TICKS(HIGH_PERIOD_MS);
    configASSERT(period > 0U);

    /* High runs first at startup: block briefly so Low can take the resource.
     * Mid has a 2-tick offset, so High first requests it before Mid wakes. */
    vTaskDelay(pdMS_TO_TICKS(1U));
    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        /* TODO 5: toggle a GPIO high (marks "task_high wants the resource"),
         *         take s_resource (portMAX_DELAY), toggle a second GPIO
         *         transition (marks "task_high got it"), do a small amount
         *         of work, give s_resource back, lower the first GPIO.
         *         The gap between the two GPIO edges on the logic analyzer
         *         is your block time t1 - t0. */
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET); /* t0: request. */

        if (xSemaphoreTake(s_resource, portMAX_DELAY) == pdTRUE) {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET); /* t1: acquired. */

            busy_work_ms(1U);

            BaseType_t result = xSemaphoreGive(s_resource);
            configASSERT(result == pdTRUE);
        } else {
            /* Failed request: do not use or Give a resource not acquired. */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
        }

        vTaskDelayUntil(&last_wake, period);
    }
}
