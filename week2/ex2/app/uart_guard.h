#ifndef UART_GUARD_H
#define UART_GUARD_H

#include <stddef.h>
#include <stdint.h>

/* 0 = none (the bug), 1 = mutex, 2 = critical section, 3 = gatekeeper task.
 * Build this file four times over the exercise, once per mode. */
#ifndef UART_GUARD_MODE
#define UART_GUARD_MODE 0
#endif

/* Chứa tối đa 4 bản tin đang chờ; queue đầy thì writer chờ. */
#ifndef GATEKEEPER_QUEUE_LENGTH
#define GATEKEEPER_QUEUE_LENGTH 4U
#endif

/* Mode 0/1: tạo điều kiện xen task rõ ràng, không phải cấu hình sản phẩm.
 * Đặt 0 để quan sát chỉ với preemption/time slicing tự nhiên.
 */
#ifndef UART_DEMO_YIELD_BETWEEN_BYTES
#define UART_DEMO_YIELD_BETWEEN_BYTES 1
#endif

extern volatile uint32_t uart_busy_count;
extern volatile uint32_t uart_tx_errors;

void uart_guard_init(void);

/* Blocking: sends `len` bytes out USART1 under whatever UART_GUARD_MODE picks.
 * In gatekeeper mode this enqueues to the gatekeeper instead of transmitting
 * directly — same call site either way. */
void uart_guard_send(const char *buf, size_t len);

#if UART_GUARD_MODE == 3
void gatekeeper_task(void *argument);
#endif

#endif /* UART_GUARD_H */
