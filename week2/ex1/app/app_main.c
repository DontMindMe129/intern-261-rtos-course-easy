/* Week 2 / Ex1 — sensor pipeline: sensor_task -> queue_raw -> filter_task ->
 * queue_filtered -> uart_task.
 *
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "main.h"
#include "params.h"
#include "pipeline.h"

QueueHandle_t queue_raw;
QueueHandle_t queue_filtered;
extern UART_HandleTypeDef huart1;

void app_main(void)
{
    /* Đổi baud thực tế trước khi tạo task; terminal phải chọn cùng baud. */
    if (huart1.Init.BaudRate != EX1_UART_BAUD) {
        huart1.Init.BaudRate = EX1_UART_BAUD;
        if (HAL_UART_Init(&huart1) != HAL_OK) {
            Error_Handler();
        }
    }

    /* TODO 1: xQueueCreate(QUEUE_LENGTH, sizeof(raw_sample_t)) for queue_raw,
     *         and the equivalent for queue_filtered with filtered_sample_t.
     *         Check both handles are non-NULL before continuing. */
    queue_raw = xQueueCreate(QUEUE_LENGTH, sizeof(raw_sample_t));
    queue_filtered = xQueueCreate(QUEUE_LENGTH, sizeof(filtered_sample_t));

    configASSERT(queue_raw != NULL);
    configASSERT(queue_filtered != NULL);

    /* TODO 2: xTaskCreate sensor_task, filter_task, uart_task.
     *         Pick priorities you can justify in REPORT.md: who must never be
     *         starved by whom? */

    BaseType_t result = xTaskCreate(
        sensor_task,
        "sensor",
        256U,
        NULL,
        3,
        NULL);
    configASSERT(result == pdPASS);

    result = xTaskCreate(
        uart_task,
        "uart",
        256U,
        NULL,
        1,
        NULL
    );
    configASSERT(result == pdPASS);

    result = xTaskCreate(
        filter_task,
        "filter",
        128U,
        NULL,
        2,
        NULL
    );
    configASSERT(result == pdPASS);

    /* TODO 3: xTaskCreate depth_logger_task — must not perturb the pipeline it
     *         is measuring (think about its own priority and period). */

#ifdef EX1_MEASUREMENT_ENABLED
    result = measurement_create_task();
    configASSERT(result == pdPASS);
#endif

    vTaskStartScheduler();

    /* Scheduler trả về khi không khởi động được. */
    Error_Handler();

    for (;;) {
    }
}
