#ifndef UART_RX_H
#define UART_RX_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "stream_buffer.h"

extern StreamBufferHandle_t rx_stream;
extern volatile uint32_t g_rx_stream_dropped_bytes;
extern volatile uint32_t g_rx_bytes;
extern volatile uint32_t g_uart_rx_errors;
extern volatile uint32_t g_uart_rearm_errors;

/* Creates rx_stream (STREAM_BUFFER_BYTES, trigger level TRIGGER_LEVEL).
 * Call once before vTaskStartScheduler(). */
void uart_rx_init(void);

/* Arms the first receive. Call once from parser_task after scheduler start,
 * with USART1 already initialized at RX_BAUD. */
void uart_rx_start(void);

/* Called from HAL_UART_RxCpltCallback(huart) for USART1. Pushes the one
 * received byte into rx_stream and re-arms HAL_UART_Receive_IT() for the
 * next byte. */
void uart_rx_isr_handler(void);
void uart_rx_error_handler(void);

#endif /* UART_RX_H */
