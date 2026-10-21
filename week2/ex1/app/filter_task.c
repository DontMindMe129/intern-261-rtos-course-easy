/* Week 2 / Ex1 — moving-average filter, window = FILTER_WINDOW samples.
 * queue_raw -> queue_filtered.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "main.h"
#include "params.h"
#include "pipeline.h"

/* Xem bằng debugger: số kết quả không gửi được vì queue đầy. */
volatile uint32_t filter_queue_drops = 0;

void filter_task(void *argument)
{
    (void)argument;

    /* TODO 1: ring buffers for ax/ay/az, size FILTER_WINDOW, plus running sums.
     *         Do NOT recompute the full sum every sample — that is the whole
     *         point of a moving-average window. */

    int16_t buffer_x[EX1_FILTER_WINDOW] = {0};
    int16_t buffer_y[EX1_FILTER_WINDOW] = {0};
    int16_t buffer_z[EX1_FILTER_WINDOW] = {0};

    int32_t sum_x = 0, sum_y = 0, sum_z = 0;
    uint32_t index = 0;
    uint32_t count = 0;

    configASSERT(queue_raw != NULL);
    configASSERT(queue_filtered != NULL);
    configASSERT(EX1_FILTER_WINDOW > 0);

    for (;;) {
        raw_sample_t in;

        /* TODO 2: xQueueReceive(queue_raw, &in, portMAX_DELAY). */

        /* Chờ mẫu; INCLUDE_vTaskSuspend = 1 cho phép chờ vô hạn. */
        xQueueReceive(queue_raw, &in, portMAX_DELAY);

        /* TODO 3: push into the ring buffer, update running sums, compute the
         *         averaged ax/ay/az. */

        /* Trừ mẫu cũ tại ô sắp ghi đè. */
        sum_x -= buffer_x[index];
        sum_y -= buffer_y[index];
        sum_z -= buffer_z[index];

        buffer_x[index] = in.ax;
        buffer_y[index] = in.ay;
        buffer_z[index] = in.az;

        /* Thêm mẫu mới vào tổng. */
        sum_x += in.ax;
        sum_y += in.ay;
        sum_z += in.az;

        if (count < EX1_FILTER_WINDOW) {
            count++;
        }
        index = (index + 1U) % EX1_FILTER_WINDOW;

        /* count >= 1; lúc khởi động chỉ chia cho số mẫu thực nhận. */
        filtered_sample_t out;
        out.seq = in.seq;
        /* TODO 4: fill out.ax/ay/az from the running average. */

        out.ax = (int16_t)(sum_x / (int32_t)count);
        out.ay = (int16_t)(sum_y / (int32_t)count);
        out.az = (int16_t)(sum_z / (int32_t)count);

        /* TODO 5: xQueueSend(queue_filtered, &out, <timeout>) — same question
         *         as sensor_task: what should happen here if uart_task is
         *         slow and queue_filtered is full? */

        /* Timeout theo cấu hình lượt đo; hết thời gian thì đếm mẫu bị bỏ. */
        if (xQueueSend(queue_filtered, &out, EX1_FILTER_TIMEOUT_TICKS) != pdPASS) {
            filter_queue_drops++;
        }
    }
}
