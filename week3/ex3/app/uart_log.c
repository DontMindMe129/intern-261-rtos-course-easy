#include <string.h>
#include "main.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "uart_log.h"
#include "experiment_config.h"

extern UART_HandleTypeDef huart1;
static QueueHandle_t s_tx_queue;
typedef struct { char text[EX3_TX_LINE_BYTES]; } tx_line_t;
volatile uint32_t g_tx_dropped_lines;
volatile uint32_t g_tx_errors;

void uart_log_init(void)
{
    s_tx_queue = xQueueCreate(EX3_TX_QUEUE_LENGTH, sizeof(tx_line_t));
    configASSERT(s_tx_queue != NULL);
}

void uart_log_line(const char *text)
{
    tx_line_t line = {0};
    size_t length = strlen(text);
    if (length >= sizeof(line.text)) {
        ++g_tx_dropped_lines;
        return;
    }
    memcpy(line.text, text, length);
    if (xQueueSend(s_tx_queue, &line, 0U) != pdPASS) {
        ++g_tx_dropped_lines;
    }
}

void uart_log_task(void *argument)
{
    (void)argument;
    tx_line_t line;
    for (;;) {
        if (xQueueReceive(s_tx_queue, &line, portMAX_DELAY) == pdPASS) {
            if (HAL_UART_Transmit(&huart1, (uint8_t *)line.text,
                                  (uint16_t)strlen(line.text),
                                  EX3_TX_TIMEOUT_MS) != HAL_OK) {
                ++g_tx_errors;
            }
        }
    }
}
