/* Week 2 / Ex2 — four ways to serialize access to USART1 from WRITER_COUNT
 * tasks. Only one UART_GUARD_MODE is compiled at a time.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include <string.h>

#include "main.h"
#include "params.h"
#include "uart_guard.h"
#ifdef EX2_MEASUREMENT_ENABLED
#include "measurement.h"
#endif

extern UART_HandleTypeDef huart1;
volatile uint32_t uart_busy_count = 0;
volatile uint32_t uart_tx_errors = 0;

#if UART_GUARD_MODE == 1
static SemaphoreHandle_t s_uart_mutex;
#elif UART_GUARD_MODE == 3
static QueueHandle_t s_gatekeeper_queue;
typedef struct {
    char data[MSG_LEN];
#ifdef EX2_MEASUREMENT_ENABLED
    uint32_t start_cycles;
#endif
} gatekeeper_message_t;
#endif

void uart_guard_init(void)
{
#if UART_GUARD_MODE == 1
    /* TODO 1: xSemaphoreCreateMutex(). Check for NULL. */
    s_uart_mutex = xSemaphoreCreateMutex();
    configASSERT(s_uart_mutex != NULL);
#elif UART_GUARD_MODE == 3
    /* TODO 2: create s_gatekeeper_queue (length: pick and justify in
     *         REPORT.md), item size = MSG_LEN bytes.
     *         xTaskCreate(gatekeeper_task, ..., GATEKEEPER_PRIO). */
    s_gatekeeper_queue = xQueueCreate(GATEKEEPER_QUEUE_LENGTH, sizeof(gatekeeper_message_t));
    configASSERT(s_gatekeeper_queue != NULL);
#ifdef EX2_MEASUREMENT_ENABLED
    measurement_queue_timestamp_bytes = GATEKEEPER_QUEUE_LENGTH *
                                        (sizeof(gatekeeper_message_t) - MSG_LEN);
#endif

    BaseType_t result = xTaskCreate(gatekeeper_task, "gatekeeper", 256U,
                                   NULL, GATEKEEPER_PRIO, NULL);
    configASSERT(result == pdPASS);
#endif
}

void uart_guard_send(const char *buf, size_t len)
{
#if defined(EX2_MEASUREMENT_ENABLED) && UART_GUARD_MODE != 0
    uint32_t start_cycles = measurement_now();
#endif
#if UART_GUARD_MODE == 0
    /* The bug: no serialization at all. */
    /* TODO 3: HAL_UART_Transmit directly. */
    for (size_t i = 0; i < len; i++) {
        HAL_StatusTypeDef status;
        do {
            status = HAL_UART_Transmit(&huart1, (const uint8_t *)&buf[i], 1, 20U);
            if (status == HAL_BUSY) {
                /* Chỉ bảo vệ bộ đếm, không bảo vệ việc gửi UART. */
                taskENTER_CRITICAL();
                uart_busy_count++;
                taskEXIT_CRITICAL();
                vTaskDelay(1); /* Nhường CPU; thử lại cùng byte. */
            }
        } while (status == HAL_BUSY);

        if (status != HAL_OK) {
            taskENTER_CRITICAL();
            uart_tx_errors++;
            taskEXIT_CRITICAL();
            return; /* Dòng có thể dở dang; không coi đây là lỗi xen ký tự. */
        }

#if UART_DEMO_YIELD_BETWEEN_BYTES
        /* Cố ý cho writer cùng priority chen giữa các byte của bản tin. */
        taskYIELD();
#endif
    }
    (void)buf; (void)len;

#elif UART_GUARD_MODE == 1
    /* TODO 4: xSemaphoreTake(s_uart_mutex, portMAX_DELAY), transmit, Give.
     *         Every return path must Give — no early return between Take and
     *         Give. */
    if (xSemaphoreTake(s_uart_mutex, portMAX_DELAY) != pdTRUE) {
        return; /* Chưa lấy được mutex nên không cần Give. */
    }

    BaseType_t sent = pdTRUE;
#ifdef EX2_MEASUREMENT_ENABLED
    uint32_t end_cycles = 0U;
#endif
    /* Giữ cách gửi từng byte và yield như mode 0, nhưng khóa CẢ bản tin. */
    for (size_t i = 0; i < len; i++) {
        HAL_StatusTypeDef status;
        do {
            status = HAL_UART_Transmit(&huart1, (const uint8_t *)&buf[i], 1, 20U);
            if (status == HAL_BUSY) {
                uart_busy_count++; /* Chỉ task giữ mutex mới vào được đây. */
                vTaskDelay(1);
            }
        } while (status == HAL_BUSY);

        if (status != HAL_OK) {
            uart_tx_errors++;
            sent = pdFALSE;
            break; /* Không return: phải đi xuống Give trước. */
        }

#ifdef EX2_MEASUREMENT_ENABLED
        if (i + 1U == len) {
            end_cycles = measurement_now();
        }
#endif

#if UART_DEMO_YIELD_BETWEEN_BYTES
        /* Task khác được chạy nhưng phải chờ mutex trước khi dùng UART. */
        taskYIELD();
#endif
    }

#ifdef EX2_MEASUREMENT_ENABLED
    if (sent == pdTRUE) {
        measurement_record(start_cycles, end_cycles, 0U);
    }
#endif
    (void)sent;
    BaseType_t released = xSemaphoreGive(s_uart_mutex);
    configASSERT(released == pdTRUE);
    (void)buf; (void)len;

#elif UART_GUARD_MODE == 2
    /* TODO 5: taskENTER_CRITICAL(), transmit, taskEXIT_CRITICAL().
     *         Measure in REPORT.md how long the critical section holds IRQs
     *         off, as a function of MSG_LEN and UART_BAUD. */
    configASSERT(len > 0 && len <= UINT16_MAX);

    taskENTER_CRITICAL();

#ifdef EX2_MEASUREMENT_ENABLED
    uint32_t critical_start = measurement_now();
#endif

    /* Gửi cả bản tin bằng polling; không delay/yield bên trong critical.
     * TIM4 priority 15 bị BASEPRI che: timeout HAL 20 ms không tiến triển
     * bình thường. Nếu cờ UART kẹt, hàm có thể chờ mãi; đây cũng là hạn chế
     * cần giải thích khi đánh giá critical section trong report.
     */
    HAL_StatusTypeDef status = HAL_UART_Transmit(&huart1,
        (const uint8_t *)buf, (uint16_t)len, 20U);

    if (status == HAL_BUSY) {
        uart_busy_count++;
    } else if (status != HAL_OK) {
        uart_tx_errors++;
    }

#ifdef EX2_MEASUREMENT_ENABLED
    uint32_t critical_end = measurement_now();
#endif
    taskEXIT_CRITICAL();
#ifdef EX2_MEASUREMENT_ENABLED
    if (status == HAL_OK) {
        measurement_record(start_cycles, critical_end, critical_end - critical_start);
    }
#endif
    (void)buf; (void)len;

#elif UART_GUARD_MODE == 3
    /* TODO 6: xQueueSend(s_gatekeeper_queue, buf, portMAX_DELAY).
     *         The writer never touches UART directly in this mode. */
    configASSERT(len == MSG_LEN);
    /* Queue sao chép bản tin; timestamp chỉ có trong bản build đo lường. */
#ifdef EX2_MEASUREMENT_ENABLED
    gatekeeper_message_t packet;
    memcpy(packet.data, buf, MSG_LEN);
    packet.start_cycles = start_cycles;
    BaseType_t queued = xQueueSend(s_gatekeeper_queue, &packet, portMAX_DELAY);
#else
    BaseType_t queued = xQueueSend(s_gatekeeper_queue, buf, portMAX_DELAY);
#endif
    configASSERT(queued == pdPASS);
    (void)buf; (void)len;
#endif
}

#if UART_GUARD_MODE == 3
void gatekeeper_task(void *argument)
{
    (void)argument;
    for (;;) {
        gatekeeper_message_t msg;
        /* TODO 7: xQueueReceive(s_gatekeeper_queue, msg, portMAX_DELAY), then
         *         HAL_UART_Transmit. This is the only task allowed to call
         *         HAL_UART_Transmit in this mode. */
        if (xQueueReceive(s_gatekeeper_queue, &msg, portMAX_DELAY) == pdPASS) {
            /* Gửi trọn bản tin; không có writer nào gọi HAL UART ở mode 3. */
            HAL_StatusTypeDef status = HAL_UART_Transmit(&huart1,
                (const uint8_t *)msg.data, MSG_LEN, 20U);
#ifdef EX2_MEASUREMENT_ENABLED
            uint32_t end_cycles = measurement_now();
            if (status == HAL_OK) {
                measurement_record(msg.start_cycles, end_cycles, 0U);
            }
#endif
            if (status == HAL_BUSY) {
                uart_busy_count++;
            } else if (status != HAL_OK) {
                uart_tx_errors++;
            }
        }
        (void)msg;
    }
}
#endif
