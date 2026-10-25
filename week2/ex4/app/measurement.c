#include "measurement.h"
#include "main.h"
#include "sync_tasks.h"

volatile measurement_stats_t measurement_event_latency;
volatile measurement_stats_t measurement_notify_latency;
volatile uint32_t measurement_latency_done;

static volatile measurement_kind_t s_wait_kind;
static volatile measurement_kind_t s_pending_kind;
static volatile EventBits_t s_wait_mask;
static volatile uint32_t s_start_cycles;

void measurement_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/* Called by sync_task immediately before its blocking API. With sync_task
 * above producers in priority, producers run only once it blocks. */
void measurement_wait_begin(measurement_kind_t kind, EventBits_t mask)
{
    s_pending_kind = MEASURE_NONE;
    s_wait_mask = mask;
    s_wait_kind = kind;
}

void measurement_signal_start(measurement_kind_t kind, EventGroupHandle_t events, EventBits_t bit)
{
    if (s_wait_kind != kind) {
        return;
    }

    volatile measurement_stats_t *stats = (kind == MEASURE_EVENT) ? &measurement_event_latency : &measurement_notify_latency;

    if (stats->count >= MEASUREMENT_SAMPLE_COUNT) {
        return;
    }

    if (kind == MEASURE_EVENT) {
        EventBits_t current = xEventGroupGetBits(events);
        EventBits_t mask = s_wait_mask;

        /* Only the bit completing this wait is a wake-up measurement. */
        if ((current & mask) == mask || ((current | bit) & mask) != mask) {
            return;
        }
    }

    /* A timeout can have preempted the producer during the checks above. */
    if (s_wait_kind != kind) {
        return;
    }

    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0); /* Optional t0 edge. */
    s_start_cycles = DWT->CYCCNT;
    s_pending_kind = kind;
}

void measurement_wait_end(measurement_kind_t kind, BaseType_t success)
{
    uint32_t end_cycles = DWT->CYCCNT;
    s_wait_kind = MEASURE_NONE;

    if (success != pdTRUE || s_pending_kind != kind) {
        s_pending_kind = MEASURE_NONE;
        return;
    }

    uint32_t cycles = end_cycles - s_start_cycles;
    s_pending_kind = MEASURE_NONE;
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1); /* Optional t1 edge. */

    /* Only sync_task writes the stats. Calculation happens after t1. */
    volatile measurement_stats_t *stats = (kind == MEASURE_EVENT)
        ? &measurement_event_latency : &measurement_notify_latency;

    if (stats->count < MEASUREMENT_SAMPLE_COUNT) {
        uint32_t count = stats->count + 1U;
        uint64_t total = stats->total_cycles + cycles;

        stats->total_cycles = total;
        stats->avg_cycles = (uint32_t)(total / count);

        if (cycles > stats->max_cycles) {
            stats->max_cycles = cycles;
        }

        float cycles_per_us = (float)SystemCoreClock / 1000000.0f;
        stats->avg_us = ((float)total / (float)count) / cycles_per_us;
        stats->max_us = (float)stats->max_cycles / cycles_per_us;
        stats->count = count;
    }

#if PRODUCER0_USE_NOTIFY
    if (measurement_notify_latency.count == MEASUREMENT_SAMPLE_COUNT) {
#else
    if (measurement_event_latency.count == MEASUREMENT_SAMPLE_COUNT) {
#endif
        measurement_latency_done = 1U;
    }
}
