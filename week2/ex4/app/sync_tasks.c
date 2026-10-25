/* Week 2 / Ex4 — event group sync across EVENT_BIT_COUNT producers, with
 * producer_0's signal optionally replaced by a task notification.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include <stdio.h>

#include "main.h"
#include "params.h"
#include "sync_tasks.h"
#include "measurement.h"

static EventGroupHandle_t s_events;
TaskHandle_t sync_task_handle = NULL;
extern UART_HandleTypeDef huart1;

/* RAM measurements: inspect these globals in debugger Watch. */
volatile uint32_t measurement_event_group_struct_bytes;
volatile uint32_t measurement_event_group_heap_bytes;
volatile uint32_t measurement_notify_heap_bytes;
volatile uint32_t measurement_notify_ram_done;
volatile uint32_t measurement_test_case;
volatile uint32_t measurement_producer_count;
volatile uint32_t measurement_sync_count;
volatile uint32_t measurement_timeout_count;

#define ALL_BITS_MASK ((1U << EX4_PRODUCER_COUNT) - 1U)

void sync_init(void)
{
    measurement_init();
    measurement_test_case = EX4_TEST_CASE;
    measurement_producer_count = EX4_PRODUCER_COUNT;

    /* TODO 1: xEventGroupCreate(). Check for NULL.
     *         Measure and record in REPORT.md the sizeof the structure this
     *         allocates (see event_groups.c in the kernel sources) to answer
     *         PREDICTION.md question 1. */
    /* heap_4 returns 0 free bytes until its first allocation. */
    void *heap_probe = pvPortMalloc(1U);
    configASSERT(heap_probe != NULL);
    vPortFree(heap_probe);

    size_t heap_before = xPortGetFreeHeapSize();

    s_events = xEventGroupCreate();
    configASSERT(s_events != NULL);

    measurement_event_group_heap_bytes = (uint32_t)(heap_before - xPortGetFreeHeapSize());

    /* Public type guaranteed by FreeRTOS to match the internal object size.
     * sizeof does not allocate an additional event group. */
    measurement_event_group_struct_bytes = sizeof(StaticEventGroup_t);
}

void producer_task(void *argument)
{
    uint32_t idx = (uint32_t)(uintptr_t)argument;
    uint32_t random_state = 0x9E3779B9U * (idx + 1U);

    configASSERT(idx < EX4_PRODUCER_COUNT);
    configASSERT(NOTIFY_PERIOD_MS > 0);
    configASSERT(pdMS_TO_TICKS(NOTIFY_PERIOD_MS) > 0U);

    for (;;) {
        /* TODO 2: random small period (reuse a simple xorshift like ex3's
         *         periodic task, or vTaskDelay with a bounded random jitter). */
        /* Each task has its own reproducible pseudo-random sequence. */
        random_state ^= random_state << 13;
        random_state ^= random_state >> 17;
        random_state ^= random_state << 5;
        uint32_t period_ms = NOTIFY_PERIOD_MS + random_state % (NOTIFY_PERIOD_MS + 1U);

#if PRODUCER0_USE_NOTIFY
        if (idx == 0U) {
            period_ms = EX4_NOTIFY_PRODUCER_PERIOD_MS;
        }
#endif

#if EX4_TEST_CASE == EX4_TIMEOUT3
        /* No delay while holding any resource: only producer_2 is slowed. */
        if (idx == EX4_SLOW_PRODUCER_ID) {
            period_ms = EX4_SLOW_PRODUCER_DELAY_MS;
        }
#endif

        vTaskDelay(pdMS_TO_TICKS(period_ms));

#if PRODUCER0_USE_NOTIFY
        if (idx == 0) {
            /* TODO 3: xTaskNotifyGive(sync_task_handle).
             *         Toggle a DWT-timestamped GPIO edge right before this
             *         call — this is your t0 for the wake-up latency
             *         measurement. */
            configASSERT(sync_task_handle != NULL);

            BaseType_t measure_heap = (measurement_notify_ram_done == 0U) ? pdTRUE : pdFALSE;
            size_t heap_before_notify = 0U;
            if (measure_heap == pdTRUE) {
                heap_before_notify = xPortGetFreeHeapSize();
            }

            measurement_signal_start(MEASURE_NOTIFY, NULL, 0U);
            BaseType_t result = xTaskNotifyGive(sync_task_handle);
            configASSERT(result == pdPASS);

            if (measure_heap == pdTRUE) {
                measurement_notify_heap_bytes = (uint32_t)(heap_before_notify - xPortGetFreeHeapSize());
                measurement_notify_ram_done = 1U;
            }
        } else
#endif
        {
            /* TODO 4: xEventGroupSetBits(s_events, 1u << idx).
             *         Same timestamp discipline as TODO 3 for producers that
             *         still use the event group. */
            /* Report compares event-group signals from producer_1/2. */
            if (idx == 1U || idx == 2U) {
                measurement_signal_start(MEASURE_EVENT, s_events, (EventBits_t)(1U << idx));
            }

            (void)xEventGroupSetBits(s_events, (EventBits_t)(1U << idx));
        }
    }
}

void sync_task(void *argument)
{
    (void)argument;

    const TickType_t timeout = pdMS_TO_TICKS(SYNC_TIMEOUT_MS);
    TickType_t previous_sync = 0U;
    BaseType_t have_previous_sync = pdFALSE;

    configASSERT(timeout > 0U);

    for (;;) {
        EventBits_t bits = 0U;
        uint32_t notify_count = 0U;
        BaseType_t synced = pdFALSE;

#if PRODUCER0_USE_NOTIFY
        /* TODO 5: ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(SYNC_TIMEOUT_MS)) for
         *         producer_0's contribution, THEN xEventGroupWaitBits for the
         *         remaining producers' bits (mask without bit 0), clear-on-exit
         *         = pdTRUE, wait-for-all = pdTRUE, same timeout.
         *         Toggle a GPIO edge the moment both are satisfied — your t1
         *         for the latency measurement. */
        const EventBits_t remaining_mask = (EventBits_t)(ALL_BITS_MASK & ~1U);

        measurement_wait_begin(MEASURE_NOTIFY, 0U);
        notify_count = ulTaskNotifyTake(pdTRUE, timeout);
        measurement_wait_end(MEASURE_NOTIFY, (notify_count > 0U) ? pdTRUE : pdFALSE);

        if (notify_count > 0U) {
            measurement_wait_begin(MEASURE_EVENT, remaining_mask);
            bits = xEventGroupWaitBits(s_events, remaining_mask, pdTRUE, pdTRUE, timeout);

            measurement_wait_end(MEASURE_EVENT, ((bits & remaining_mask) == remaining_mask) ? pdTRUE : pdFALSE);

            synced = ((bits & remaining_mask) == remaining_mask) ? pdTRUE : pdFALSE;
        } else {
            /* Snapshot for the log; no event-group wait was performed. */
            bits = xEventGroupGetBits(s_events);
        }
#else
        /* TODO 6: xEventGroupWaitBits(s_events, ALL_BITS_MASK, pdTRUE, pdTRUE,
         *         pdMS_TO_TICKS(SYNC_TIMEOUT_MS)).
         *         Check the return value against ALL_BITS_MASK to tell
         *         "all bits arrived" apart from "timed out with some bits
         *         missing" — do not assume a non-zero return means success. */
        measurement_wait_begin(MEASURE_EVENT, (EventBits_t)ALL_BITS_MASK);
        bits = xEventGroupWaitBits(s_events, (EventBits_t)ALL_BITS_MASK, pdTRUE, pdTRUE, timeout);

        measurement_wait_end(MEASURE_EVENT, ((bits & ALL_BITS_MASK) == ALL_BITS_MASK) ? pdTRUE : pdFALSE);

        synced = ((bits & ALL_BITS_MASK) == ALL_BITS_MASK) ? pdTRUE : pdFALSE;
#endif

        /* TODO 7: print the sync instant and measured period to UART. */
        TickType_t now = xTaskGetTickCount();
        char line[112];
        int length;

        if (synced == pdTRUE) {
            measurement_sync_count++;
            TickType_t period_ticks = have_previous_sync ? (now - previous_sync) : 0U;

            previous_sync = now;
            have_previous_sync = pdTRUE;

            length = snprintf(line, sizeof(line),
                              "SYNC t=%lums period=%lums bits=0x%06lx notify=%lu\r\n",
                              (unsigned long)(now * portTICK_PERIOD_MS),
                              (unsigned long)(period_ticks * portTICK_PERIOD_MS),
                              (unsigned long)bits, (unsigned long)notify_count);
        } else {
            measurement_timeout_count++;
            length = snprintf(line, sizeof(line),
                              "TIMEOUT t=%lums bits=0x%06lx notify=%lu\r\n",
                              (unsigned long)(now * portTICK_PERIOD_MS),
                              (unsigned long)bits, (unsigned long)notify_count);
        }

        if (length < 0 || (size_t)length >= sizeof(line)) {
            Error_Handler();
        }

        HAL_StatusTypeDef status = HAL_UART_Transmit(&huart1, (const uint8_t *)line, (uint16_t)length, 20U);

        if (status != HAL_OK) {
            Error_Handler();
        }
    }
}
