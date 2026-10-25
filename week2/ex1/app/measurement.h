#ifndef EX1_MEASUREMENT_H
#define EX1_MEASUREMENT_H

#include <stdint.h>
#include "FreeRTOS.h"

#define MEASUREMENT_COUNT 256U
#define MEASUREMENT_PERIOD_MS 2U

/* Mỗi dòng chỉ có thời gian và độ đầy hai queue. */
typedef struct {
    uint32_t time_ms;
    uint32_t raw_depth;
    uint32_t filtered_depth;
} QueueLog;

typedef struct {
    uint32_t sensor_read_errors;
    uint32_t sensor_queue_drops;
    uint32_t filter_queue_drops;
    uint32_t uart_format_errors;
    uint32_t uart_transmit_errors;
} ErrorLog;

extern volatile QueueLog measurement_log[MEASUREMENT_COUNT];
extern volatile uint32_t measurement_count;
extern volatile uint32_t measurement_done;
extern volatile ErrorLog measurement_errors;

BaseType_t measurement_create_task(void);
void measurement_begin(void);

/* Optional pipeline latency test: L16 / N32 only. */
typedef struct {
    uint32_t count;
    uint64_t total_cycles;
    uint32_t min_cycles;
    uint32_t max_cycles;
    float avg_us;
    float min_us;
    float max_us;
} PipelineLatency;

extern volatile PipelineLatency measurement_latency;
extern volatile uint32_t measurement_latency_done;
extern volatile uint32_t measurement_latency_window;

void measurement_latency_init(void);
uint32_t measurement_latency_now(void);
void measurement_latency_record(uint32_t read_start_cycles, uint32_t window_full);

/* Controlled step response: one 0 -> 1000 transition, threshold 500. */
typedef struct {
    uint32_t window;
    uint32_t start_seq;
    uint32_t threshold_seq;
    uint32_t start_cycles;
    uint32_t end_cycles;
    float response_ms;
    uint32_t valid;
    uint32_t done;
} StepResponse;

extern volatile StepResponse measurement_step;

void measurement_step_init(void);
void measurement_step_begin(uint32_t seq);
void measurement_step_record(uint32_t seq, int16_t filtered_ax);

#endif /* EX1_MEASUREMENT_H */
