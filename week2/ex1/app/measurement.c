/* Chỉ ghi độ đầy queue vào RAM; không in thêm UART lúc đo. */
#include "FreeRTOS.h"
#include "task.h"
#include "pipeline.h"
#include "main.h"

_Static_assert(sizeof(QueueLog) == 12, "Update Python parser if layout changes");

volatile QueueLog measurement_log[MEASUREMENT_COUNT];
volatile uint32_t measurement_count = 0;
volatile uint32_t measurement_done = 0;
volatile ErrorLog measurement_errors;

extern volatile uint32_t sensor_read_errors, sensor_queue_drops;
extern volatile uint32_t filter_queue_drops;
extern volatile uint32_t uart_format_errors, uart_transmit_errors;

static volatile uint32_t started = 0;

void measurement_begin(void)
{
    started = 1; /* Sensor đã khởi tạo xong. */
}

void depth_logger_task(void *argument)
{
    (void)argument;
    while (!started) {
        vTaskDelay(1);
    }

    TickType_t start = xTaskGetTickCount();
    TickType_t last_wake = start;
    TickType_t period = pdMS_TO_TICKS(MEASUREMENT_PERIOD_MS);
    configASSERT(period > 0);

    for (uint32_t i = 0; i < MEASUREMENT_COUNT; i++) {
        measurement_log[i].time_ms =
            (uint32_t)(xTaskGetTickCount() - start) * 1000U / configTICK_RATE_HZ;
        measurement_log[i].raw_depth = uxQueueMessagesWaiting(queue_raw);
        measurement_log[i].filtered_depth = uxQueueMessagesWaiting(queue_filtered);
        measurement_count = i + 1;

        if (i + 1 < MEASUREMENT_COUNT) {
            vTaskDelayUntil(&last_wake, period);
        }
    }

    /* Giữ bộ đếm tại cuối capture để xem trong Watch. */
    measurement_errors.sensor_read_errors = sensor_read_errors;
    measurement_errors.sensor_queue_drops = sensor_queue_drops;
    measurement_errors.filter_queue_drops = filter_queue_drops;
    measurement_errors.uart_format_errors = uart_format_errors;
    measurement_errors.uart_transmit_errors = uart_transmit_errors;
    measurement_done = 1;

    vTaskSuspend(NULL); /* Giữ log nguyên vẹn; pipeline vẫn tiếp tục chạy. */
    for (;;) { }
}

BaseType_t measurement_create_task(void)
{
    /* Cao hơn ba task chính, chỉ đọc nhanh rồi nhường CPU 2 ms. */
    return xTaskCreate(depth_logger_task, "depth", 128U, NULL, 4U, NULL);
}

#if EX1_LATENCY_ENABLED

#define LATENCY_SAMPLE_COUNT 200U

volatile PipelineLatency measurement_latency;
volatile uint32_t measurement_latency_done;
volatile uint32_t measurement_latency_window;

void measurement_latency_init(void)
{
    measurement_latency_window = EX1_FILTER_WINDOW;
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t measurement_latency_now(void)
{
    return DWT->CYCCNT;
}

void measurement_latency_record(uint32_t read_start_cycles, uint32_t window_full)
{
    uint32_t end_cycles = DWT->CYCCNT;
    if (window_full == 0U || measurement_latency_done != 0U) {
        return;
    }

    uint32_t cycles = end_cycles - read_start_cycles;
    uint32_t count = measurement_latency.count;
    uint64_t total = measurement_latency.total_cycles + cycles;

    if (count == 0U || cycles < measurement_latency.min_cycles) {
        measurement_latency.min_cycles = cycles;
    }
    if (cycles > measurement_latency.max_cycles) {
        measurement_latency.max_cycles = cycles;
    }

    count++;
    measurement_latency.total_cycles = total;
    float cycles_per_us = (float)SystemCoreClock / 1000000.0f;
    measurement_latency.avg_us = ((float)total / (float)count) / cycles_per_us;
    measurement_latency.min_us = (float)measurement_latency.min_cycles / cycles_per_us;
    measurement_latency.max_us = (float)measurement_latency.max_cycles / cycles_per_us;
    measurement_latency.count = count;

    if (count == LATENCY_SAMPLE_COUNT) {
        measurement_latency_done = 1U;
    }
}

#endif

#if EX1_STEP_ENABLED

volatile StepResponse measurement_step;
static uint32_t step_started;

void measurement_step_init(void)
{
    measurement_step.window = EX1_FILTER_WINDOW;
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void measurement_step_begin(uint32_t seq)
{
    if (seq == EX1_STEP_AT_SEQ && step_started == 0U) {
        measurement_step.start_seq = seq;
        measurement_step.start_cycles = DWT->CYCCNT;
        step_started = 1U;
    }
}

void measurement_step_record(uint32_t seq, int16_t filtered_ax)
{
    uint32_t end_cycles = DWT->CYCCNT;
    if (step_started == 0U || measurement_step.done != 0U || filtered_ax < EX1_STEP_THRESHOLD) {
        return;
    }

    measurement_step.threshold_seq = seq;
    measurement_step.end_cycles = end_cycles;
    uint32_t cycles = end_cycles - measurement_step.start_cycles;
    measurement_step.response_ms = (float)cycles / ((float)SystemCoreClock / 1000.0f);

    /* With no missing samples, 50% is reached by the N/2-th HIGH sample. */
    measurement_step.valid =
        seq == EX1_STEP_AT_SEQ + EX1_FILTER_WINDOW / 2U - 1U &&
        sensor_read_errors == 0U && sensor_queue_drops == 0U &&
        filter_queue_drops == 0U && uart_format_errors == 0U && uart_transmit_errors == 0U;
    measurement_step.done = 1U;
}

#endif
