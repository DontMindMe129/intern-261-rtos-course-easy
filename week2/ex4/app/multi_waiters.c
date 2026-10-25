#include "experiment_config.h"
#include "multi_waiters.h"

#if EX4_TEST_CASE == EX4_MULTI_WAITERS

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "main.h"
#include <stdio.h>

#define COMMON_BIT       ((EventBits_t)(1U << 0))
#define READY_MASK       ((EventBits_t)((1U << 1) | (1U << 2)))
#define EVENT_DONE_MASK  ((EventBits_t)((1U << 3) | (1U << 4)))
#define NOTIFY_DONE_MASK ((EventBits_t)((1U << 5) | (1U << 6)))

static EventGroupHandle_t s_test_events;
static TaskHandle_t s_receiver_a;
extern UART_HandleTypeDef huart1;

volatile multi_waiters_result_t measurement_multi_waiters;

static void receiver_task(void *argument)
{
    uint32_t id = (uint32_t)(uintptr_t)argument;
    const TickType_t timeout = pdMS_TO_TICKS(SYNC_TIMEOUT_MS);

    /* Receivers have priority 2, controller priority 1: after setting READY,
     * each receiver reaches its wait before the controller can signal. */
    (void)xEventGroupSetBits(s_test_events, (EventBits_t)(1U << (1U + id)));
    EventBits_t bits = xEventGroupWaitBits(s_test_events, COMMON_BIT, pdTRUE, pdTRUE, timeout);

    if (id == 0U) {
        measurement_multi_waiters.event_a = ((bits & COMMON_BIT) != 0U);
    } else {
        measurement_multi_waiters.event_b = ((bits & COMMON_BIT) != 0U);
    }

    (void)xEventGroupSetBits(s_test_events, (EventBits_t)(1U << (3U + id)));
    uint32_t count = ulTaskNotifyTake(pdTRUE, timeout);

    if (id == 0U) {
        measurement_multi_waiters.notify_a = (count > 0U);
    } else {
        measurement_multi_waiters.notify_b = (count > 0U);
        measurement_multi_waiters.timeout_b = (count == 0U);
    }

    (void)xEventGroupSetBits(s_test_events, (EventBits_t)(1U << (5U + id)));
    vTaskSuspend(NULL);
    for (;;) {
    }
}

static void controller_task(void *argument)
{
    (void)argument;
    const TickType_t timeout = pdMS_TO_TICKS(2U * SYNC_TIMEOUT_MS);

    EventBits_t ready = xEventGroupWaitBits(s_test_events, READY_MASK, pdTRUE, pdTRUE, timeout);
    if ((ready & READY_MASK) == READY_MASK) {
        /* One SetBits call must wake BOTH receivers, even with clear-on-exit. */
        (void)xEventGroupSetBits(s_test_events, COMMON_BIT);

        EventBits_t event_done = xEventGroupWaitBits(s_test_events, EVENT_DONE_MASK, pdTRUE, pdTRUE, timeout);
        if ((event_done & EVENT_DONE_MASK) == EVENT_DONE_MASK) {
            /* Both receivers are now blocked on notification. Signal A only. */
            BaseType_t result = xTaskNotifyGive(s_receiver_a);
            configASSERT(result == pdPASS);

            EventBits_t notify_done = xEventGroupWaitBits(s_test_events, NOTIFY_DONE_MASK, pdTRUE, pdTRUE, timeout);
            measurement_multi_waiters.passed =
                ((notify_done & NOTIFY_DONE_MASK) == NOTIFY_DONE_MASK) &&
                measurement_multi_waiters.event_a == 1U &&
                measurement_multi_waiters.event_b == 1U &&
                measurement_multi_waiters.notify_a == 1U &&
                measurement_multi_waiters.notify_b == 0U &&
                measurement_multi_waiters.timeout_b == 1U;
        }
    }

    measurement_multi_waiters.done = 1U;

    /* Test runs once; repeat its result so opening the terminal late is OK. */
    for (;;) {
        char line[112];
        int length = snprintf(line, sizeof(line),
                              "MULTI event_A=%lu event_B=%lu notify_A=%lu notify_B=%lu timeout_B=%lu pass=%lu\r\n",
                              (unsigned long)measurement_multi_waiters.event_a,
                              (unsigned long)measurement_multi_waiters.event_b,
                              (unsigned long)measurement_multi_waiters.notify_a,
                              (unsigned long)measurement_multi_waiters.notify_b,
                              (unsigned long)measurement_multi_waiters.timeout_b,
                              (unsigned long)measurement_multi_waiters.passed);
        if (length < 0 || (size_t)length >= sizeof(line)) {
            Error_Handler();
        }
        if (HAL_UART_Transmit(&huart1, (const uint8_t *)line, (uint16_t)length, 20U) != HAL_OK) {
            Error_Handler();
        }
        vTaskDelay(pdMS_TO_TICKS(1000U));
    }
}

void multi_waiters_init(void)
{
    configASSERT(pdMS_TO_TICKS(SYNC_TIMEOUT_MS) > 0U);
    s_test_events = xEventGroupCreate();
    configASSERT(s_test_events != NULL);

    BaseType_t result = xTaskCreate(receiver_task, "receiver_A", 256U, (void *)(uintptr_t)0U, 2U, &s_receiver_a);
    configASSERT(result == pdPASS);
    result = xTaskCreate(receiver_task, "receiver_B", 256U, (void *)(uintptr_t)1U, 2U, NULL);
    configASSERT(result == pdPASS);
    result = xTaskCreate(controller_task, "controller", 256U, NULL, 1U, NULL);
    configASSERT(result == pdPASS);
}

#endif
