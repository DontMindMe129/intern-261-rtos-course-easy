/* Week 3 / Ex1 -- EXTI button wakes a task via task notification.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "exti_button.h"
#include "experiment_config.h"

static TaskHandle_t s_ack_task_handle;

void exti_button_init(void)
{
    /* TODO 1: GPIO init for PB<EXTI_PIN> as EXTI falling edge (GPIO_MODE_IT_FALLING,
     *         pull-up), and PA<ACK_PIN> as push-pull output.
     *
     * TODO 2 (do this FIRST, on purpose): HAL_NVIC_SetPriority(EXTIx_IRQn, <n>, 0)
     *         with <n> deliberately SMALLER (= higher hardware priority) than
     *         configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY. Run step 4 of the
     *         README with this wrong value first and capture the configASSERT.
     *         Only after that, set it to ISR_NVIC_PRIO and HAL_NVIC_EnableIRQ. */
    GPIO_InitTypeDef gpio = {0};

    /* Enable the IRQ later, once ack_task and the scheduler are running. */
    HAL_NVIC_DisableIRQ(EX1_BUTTON_IRQn);

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    /* PA7 starts LOW; ack_task will generate a HIGH pulse on wakeup. */
    HAL_GPIO_WritePin(EX1_ACK_PORT, EX1_ACK_PIN, GPIO_PIN_RESET);
    gpio.Pin = EX1_ACK_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(EX1_ACK_PORT, &gpio);

    /* PB1: released = HIGH, pressed = LOW (button connected to GND). */
    gpio.Pin = EX1_BUTTON_PIN;
    gpio.Mode = GPIO_MODE_IT_FALLING;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(EX1_BUTTON_PORT, &gpio);

    /* Select priority and yield behavior in experiment_config.h. Priority 4 is invalid for FromISR APIs
     * with the current FreeRTOS threshold of 5. Capture configASSERT, then
     * select EX1_TEST_YIELD for the valid experiment/final code. */
    HAL_NVIC_SetPriority(EX1_BUTTON_IRQn, EX1_EXTI_PRIORITY, EX1_EXTI_SUBPRIORITY);

    __HAL_GPIO_EXTI_CLEAR_IT(EX1_BUTTON_PIN);
    HAL_NVIC_ClearPendingIRQ(EX1_BUTTON_IRQn);
}

void exti_button_isr_handler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* TODO 3: vTaskNotifyGiveFromISR(s_ack_task_handle, &xHigherPriorityTaskWoken).
     *
     * TODO 4: portYIELD_FROM_ISR(xHigherPriorityTaskWoken) -- the README asks
     *         you to temporarily remove exactly this line in step 6 to measure
     *         the latency penalty. Do NOT remove it in the version you submit. */
    vTaskNotifyGiveFromISR(s_ack_task_handle, &xHigherPriorityTaskWoken);

#if EX1_USE_ISR_YIELD
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif
    
}

void ack_task(void *argument)
{
    (void)argument;
    s_ack_task_handle = xTaskGetCurrentTaskHandle();

    /* Task handle hợp lệ và scheduler đã chạy: có thể bật IRQ. */
    __HAL_GPIO_EXTI_CLEAR_IT(EX1_BUTTON_PIN);
    HAL_NVIC_ClearPendingIRQ(EX1_BUTTON_IRQn);
    HAL_NVIC_EnableIRQ(EX1_BUTTON_IRQn);

    for (;;) {
        /* TODO 5: ulTaskNotifyTake(pdTRUE, portMAX_DELAY). */

        /* TODO 6: toggle PA<ACK_PIN> here, nothing before it in this loop body --
         *         any extra work here inflates the latency you are measuring. */

        /**/
        ulTaskNotifyTake(pdTRUE, EX1_NOTIFY_WAIT_TICKS);

        HAL_GPIO_WritePin(EX1_ACK_PORT, EX1_ACK_PIN, GPIO_PIN_SET);

        vTaskDelay(pdMS_TO_TICKS(EX1_ACK_PULSE_MS));

        HAL_GPIO_WritePin(EX1_ACK_PORT, EX1_ACK_PIN, GPIO_PIN_RESET);

    }
}
