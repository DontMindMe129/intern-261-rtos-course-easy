/* Week 1 / Ex3 — periodic task: vTaskDelay vs xTaskDelayUntil.
 *
 * Build this file twice over the exercise, switching DELAY_MODE, and twice more
 * with configTICK_RATE_HZ changed in FreeRTOSConfig.h. Four runs total.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "dwt.h"

/* 0 = vTaskDelay, 1 = xTaskDelayUntil */
#ifndef DELAY_MODE
#define DELAY_MODE 1
#endif

/* Fixed seed: the variable load must be reproducible across all four runs,
 * otherwise the four configurations are not comparable. */
static uint32_t s_rand_state = 0x12345678u;
static uint32_t noBlockCount = 0;
static uint32_t sampleCount = 0;

static uint32_t next_work_us(void)
{
    /* TODO 1: xorshift32 on s_rand_state, mapped into [0, WORK_JITTER_US].
     *         Must give the identical sequence on every run. */
    s_rand_state ^= s_rand_state << 13;
    s_rand_state ^= s_rand_state >> 17;
    s_rand_state ^= s_rand_state << 5;
    return s_rand_state % (WORK_JITTER_US + 1U);
}

void periodic_task(void *argument)
{
    (void)argument;

#if DELAY_MODE == 1
    /* TODO 2: declare and initialise the previous-wake-time variable.
     *         Initialising it in the wrong place is the classic mistake here —
     *         if your drift is not ~0, look at this first and say so in REPORT.md. */
    TickType_t previous_wakeup_tick = xTaskGetTickCount();
#endif

    for (;;) {
        /* TODO 3: toggle PA<PERIODIC_PIN>, then burn next_work_us() of CPU
         *         with the DWT busy-wait from ex2. */
        HAL_GPIO_TogglePin(GPIOA, 1U << PERIODIC_PIN);
        sampleCount++;
        dwt_busy_wait_us(next_work_us());

#if DELAY_MODE == 0
        /* TODO 4a: vTaskDelay(...) — what exactly do you pass, and relative to what? */
        vTaskDelay(pdMS_TO_TICKS(PERIOD_MS));

        
#else
        /* TODO 4b: xTaskDelayUntil(...) — note it returns a value. What does it mean? */
        if(xTaskDelayUntil(&previous_wakeup_tick, pdMS_TO_TICKS(PERIOD_MS)) == pdFALSE){
            if(sampleCount <= SAMPLE_COUNT){
                noBlockCount++;
            }
        }
#endif
    }
}
