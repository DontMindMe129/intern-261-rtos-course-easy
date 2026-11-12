/* Week 3 / Ex3 -- UART RX interrupt -> stream buffer.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

#include "main.h"
#include "params.h"
#include "uart_rx.h"
#include "experiment_config.h"

StreamBufferHandle_t rx_stream;

/* Inspect in debugger when checking for stream-buffer overflow. */
volatile uint32_t g_rx_stream_dropped_bytes = 0U;
volatile uint32_t g_rx_bytes;
volatile uint32_t g_uart_rx_errors;
volatile uint32_t g_uart_rearm_errors;

static uint8_t s_rx_byte;

extern UART_HandleTypeDef huart1;

void uart_rx_init(void)
{
    /* TODO 1: rx_stream = xStreamBufferCreate(STREAM_BUFFER_BYTES,
     *         TRIGGER_LEVEL). Check non-NULL.
     *
     * TODO 2: HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1) to arm the first
     *         byte. huart1 must already be MX_USART1_UART_Init()'d at
     *         RX_BAUD -- that part lives in template/, not here. */
    rx_stream = xStreamBufferCreate(EX3_STREAM_BYTES, TRIGGER_LEVEL);
    configASSERT(rx_stream != NULL);
    /* TODO 2 is performed by uart_rx_start() from parser_task, after the
     * scheduler starts, so the RX ISR cannot call FreeRTOS too early. */
}

void uart_rx_start(void)
{
    configASSERT(rx_stream != NULL);
    HAL_StatusTypeDef result = HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1U);
    configASSERT(result == HAL_OK);
}

void uart_rx_isr_handler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    ++g_rx_bytes;

    /* TODO 3: xStreamBufferSendFromISR(rx_stream, &s_rx_byte, 1,
     *         &xHigherPriorityTaskWoken). Check the return value: what do you
     *         do with the byte if the stream buffer is full and it returns 0?
     *         Answer this in PREDICTION.md, not just here. */
    size_t sent = xStreamBufferSendFromISR(rx_stream, &s_rx_byte, 1U, &xHigherPriorityTaskWoken);
    if (sent != 1U) {
        /* Buffer full: discard this byte and keep receiving. Never wait
         * or print from the ISR. Count the loss for the measurement. */
        ++g_rx_stream_dropped_bytes;
    }

    /* TODO 4: HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1) again -- re-arm for
     *         the next byte. Forgetting this is the single most common way
     *         to silently stop receiving after the first byte. */
    HAL_StatusTypeDef result = HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1U);
    configASSERT(result == HAL_OK);

    /* TODO 5: portYIELD_FROM_ISR(xHigherPriorityTaskWoken). */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void uart_rx_error_handler(void)
{
    ++g_uart_rx_errors;
    /* HAL ends interrupt-mode RX on overrun. Non-blocking errors can leave
     * RX active, so only re-arm if HAL reports it is ready. */
    if (huart1.RxState == HAL_UART_STATE_READY) {
        __HAL_UART_CLEAR_OREFLAG(&huart1);
        if (HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1U) != HAL_OK) {
            ++g_uart_rearm_errors;
        }
    }
}
