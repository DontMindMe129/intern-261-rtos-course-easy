/* Week 3 / Ex2 -- software timer debounce.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "debounce.h"

static void init_task(void *argument)
{
    (void)argument;

    /* Timers are created before the scheduler starts. Discard old edges
     * and only enable the button interrupt from task context. */
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
    HAL_NVIC_ClearPendingIRQ(EXTI1_IRQn);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);

    vTaskDelete(NULL);
}

void app_main(void)
{
    HAL_NVIC_DisableIRQ(EXTI1_IRQn);

    /* TODO: GPIO init for the button pin as EXTI, both edges
     *       (GPIO_MODE_IT_RISING_FALLING) -- you need every bounce edge, not
     *       just the first one. */

    debounce_init();

    /* Priority 8 is in the permitted range for timer FromISR APIs. */
    HAL_NVIC_SetPriority(EXTI1_IRQn, 8U, 0U);

    BaseType_t result = xTaskCreate(init_task, "init", 128U, NULL, 2U, NULL);
    configASSERT(result == pdPASS);

    vTaskStartScheduler();

    Error_Handler();

    for (;;) {
    }
}
