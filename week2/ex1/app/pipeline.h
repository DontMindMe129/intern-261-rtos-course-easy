#ifndef PIPELINE_H
#define PIPELINE_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "params.h"

/* Có file thử nghiệm cục bộ thì dùng; thiếu file vẫn build baseline. */
#if __has_include("experiment_config.h")
#include "experiment_config.h"
#endif

#ifndef EX1_UART_BAUD
#define EX1_UART_BAUD UART_BAUD
#endif
#ifndef EX1_FILTER_WINDOW
#define EX1_FILTER_WINDOW FILTER_WINDOW
#endif
#ifndef EX1_SENSOR_TIMEOUT_TICKS
#define EX1_SENSOR_TIMEOUT_TICKS 0U
#endif
#ifndef EX1_FILTER_TIMEOUT_TICKS
#define EX1_FILTER_TIMEOUT_TICKS 0U
#endif

#ifdef EX1_MEASUREMENT_ENABLED
#include "measurement.h"
#else
#define measurement_begin() ((void)0)
#endif

/* One raw sample from MPU6050, passed sensor_task -> filter_task. */
typedef struct {
    uint32_t seq;   /* monotonically increasing — the only honest way to count
                     * dropped/late samples downstream */
    int16_t  ax, ay, az;
} raw_sample_t;

/* One filtered sample, passed filter_task -> uart_task. */
typedef struct {
    uint32_t seq;
    int16_t  ax, ay, az;
} filtered_sample_t;
QueueHandle_t
extern  queue_raw;
extern QueueHandle_t queue_filtered;

void sensor_task(void *argument);
void filter_task(void *argument);
void uart_task(void *argument);

/* Periodic queue-depth sampler used to produce the fill-over-time plot.
 * TODO: decide and document where this stores/sends its samples. */
void depth_logger_task(void *argument);

#endif /* PIPELINE_H */
