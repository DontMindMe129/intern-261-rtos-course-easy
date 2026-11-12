#ifndef EX3_UART_LOG_H
#define EX3_UART_LOG_H
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
extern volatile uint32_t g_tx_dropped_lines;
extern volatile uint32_t g_tx_errors;
void uart_log_init(void);
void uart_log_task(void *argument);
void uart_log_line(const char *text);
#endif
