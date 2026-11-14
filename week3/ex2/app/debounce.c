/* Week 3 / Ex2 -- debounce via one-shot timer + long-press via auto-reload
 * timer. Native FreeRTOS API only.
 */
#include <stdio.h>

#include "FreeRTOS.h"
#include "timers.h"

#include "main.h"
#include "params.h"
#include "debounce.h"
#include "experiment_config.h"

#include "task.h"

static TimerHandle_t s_debounce_timer;
static TimerHandle_t s_longpress_timer;

extern UART_HandleTypeDef huart1;

/* Pull-up: HIGH = released. Only timer callbacks update this state. */
static GPIO_PinState previous_state = GPIO_PIN_SET;

static void debounce_callback(TimerHandle_t xTimer)
{
    TickType_t current_tick = xTaskGetTickCount();
    (void)xTimer;

    /* TODO 1: printf/HAL_UART "PRESS" + xTaskGetTickCount(). This is the
     *         stable-press event, DEBOUNCE_MS after the last edge.
     *
     * TODO 2 (temporary, for the README step-4 experiment ONLY): add
     *         vTaskDelay(pdMS_TO_TICKS(50)) here, capture what happens to
     *         longpress_timer's callback timing in your REPORT.md, then
     *         DELETE this line again before you submit. A callback that
     *         blocks here is blocking the ONE daemon task every other
     *         software timer in the system also depends on. */

    GPIO_PinState current_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);

    if(current_state != previous_state){
        /* Debounced output: HIGH = pressed, LOW = released. */
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7,
                          current_state == GPIO_PIN_RESET ? GPIO_PIN_SET : GPIO_PIN_RESET);
        if(current_state == GPIO_PIN_RESET){
            char tx_buffer[128];
            int length = snprintf(tx_buffer, sizeof(tx_buffer), "PRESS, t = %lu\r\n", (unsigned long)current_tick);
            if (length > 0 && length < (int)sizeof(tx_buffer)) {
                HAL_UART_Transmit(&huart1, (uint8_t *)tx_buffer, (uint16_t)length, 20U);
            }
        }
        previous_state = current_state;
    }

    /* TODO 2 experiment only: normal builds contain no vTaskDelay here.
     * Delay after accepting the state; PA7 still marks debounce acceptance. */
#if EX2_CALLBACK_BLOCK_MS > 0
    vTaskDelay(pdMS_TO_TICKS(EX2_CALLBACK_BLOCK_MS));
#endif
}

static void longpress_callback(TimerHandle_t xTimer)
{
    TickType_t current_tick = xTaskGetTickCount();
    (void)xTimer;

    /* TODO 3: printf/HAL_UART "HELD" + xTaskGetTickCount(). Fires once per
     *         LONGPRESS_MS while the button stays down. */

    /* A stop command may still be pending when the timer expires. */
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) != GPIO_PIN_RESET) {
        return;
    }

    char tx_buffer[128];
    int length = snprintf(tx_buffer, sizeof(tx_buffer), "HELD, t = %lu\r\n", (unsigned long)current_tick);
    if (length > 0 && length < (int)sizeof(tx_buffer)) {
        HAL_UART_Transmit(&huart1, (uint8_t *)tx_buffer, (uint16_t)length, 20U);
    }

}

void debounce_init(void)
{
    /* TODO 4: s_debounce_timer = xTimerCreate(..., pdMS_TO_TICKS(DEBOUNCE_MS),
     *         pdFALSE, NULL, debounce_callback); -- pdFALSE = one-shot.
     *         s_longpress_timer = xTimerCreate(..., pdMS_TO_TICKS(LONGPRESS_MS),
     *         pdTRUE, NULL, longpress_callback); -- pdTRUE = auto-reload.
     *         Check both handles non-NULL. Do NOT xTimerStart() either here --
     *         they are driven entirely from debounce_on_edge(). */
    s_debounce_timer = xTimerCreate("debounce", pdMS_TO_TICKS(EX2_DEBOUNCE_MS), pdFALSE, NULL, debounce_callback);

    s_longpress_timer = xTimerCreate("longpress", pdMS_TO_TICKS(EX2_LONGPRESS_MS), pdTRUE, NULL, longpress_callback);

    configASSERT(s_debounce_timer != NULL);
    configASSERT(s_longpress_timer != NULL);
    
}

void debounce_on_edge(uint8_t pin_is_high)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* TODO 5: xTimerResetFromISR(s_debounce_timer, &xHigherPriorityTaskWoken)
     *         on every edge -- this is what swallows bounce: each new edge
     *         restarts the DEBOUNCE_MS countdown, so the callback only fires
     *         once the line has been quiet for DEBOUNCE_MS. */

    /* TODO 6: if (pin_is_high) xTimerStartFromISR(s_longpress_timer, ...);
     *         else xTimerStopFromISR(s_longpress_timer, ...); */

    /* TODO 7: portYIELD_FROM_ISR(xHigherPriorityTaskWoken). */
    BaseType_t result;

    result = xTimerResetFromISR(
        s_debounce_timer,
        &xHigherPriorityTaskWoken
    );
    configASSERT(result == pdPASS);

    if (pin_is_high == 0U) {
        result = xTimerStartFromISR(
            s_longpress_timer,
            &xHigherPriorityTaskWoken
        );
    } else {
        result = xTimerStopFromISR(
            s_longpress_timer,
            &xHigherPriorityTaskWoken
        );
    }
    configASSERT(result == pdPASS);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}
