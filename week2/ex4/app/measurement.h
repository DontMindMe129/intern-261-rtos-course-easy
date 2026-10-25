#ifndef EX4_MEASUREMENT_H
#define EX4_MEASUREMENT_H

#include "FreeRTOS.h"
#include "event_groups.h"
#include <stdint.h>

#define MEASUREMENT_SAMPLE_COUNT 200U

typedef enum {
    MEASURE_NONE = 0,
    MEASURE_EVENT,
    MEASURE_NOTIFY
} measurement_kind_t;

typedef struct {
    uint32_t count;
    uint64_t total_cycles;
    uint32_t avg_cycles;
    uint32_t max_cycles;
    float avg_us;
    float max_us;
} measurement_stats_t;

extern volatile measurement_stats_t measurement_event_latency;
extern volatile measurement_stats_t measurement_notify_latency;
extern volatile uint32_t measurement_latency_done;

void measurement_init(void);
void measurement_wait_begin(measurement_kind_t kind, EventBits_t mask);
void measurement_signal_start(measurement_kind_t kind, EventGroupHandle_t events, EventBits_t bit);
void measurement_wait_end(measurement_kind_t kind, BaseType_t success);

#endif
