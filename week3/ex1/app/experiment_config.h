#ifndef WEEK3_EX1_EXPERIMENT_CONFIG_H
#define WEEK3_EX1_EXPERIMENT_CONFIG_H

#include "FreeRTOS.h"
#include "params.h"

/* Chọn một test case tại đây; không sửa params.h. */
#define EX1_TEST_ASSERT       1 /* Priority sai: quan sát g_assert_failed. */
#define EX1_TEST_YIELD        2 /* Priority 8, có yield: đo latency chuẩn. */
#define EX1_TEST_NO_YIELD     3 /* Priority 8, bỏ yield: đo để so sánh. */
#define EX1_TEST_LOWER_PRIO   4 /* Priority 9, có yield: câu hỏi REPORT. */

#ifndef EX1_TEST_CASE
#define EX1_TEST_CASE EX1_TEST_YIELD
#endif

#if EX1_TEST_CASE == EX1_TEST_ASSERT
#define EX1_EXTI_PRIORITY 4U
#define EX1_USE_ISR_YIELD 1
#elif EX1_TEST_CASE == EX1_TEST_YIELD
#define EX1_EXTI_PRIORITY ISR_NVIC_PRIO
#define EX1_USE_ISR_YIELD 1
#elif EX1_TEST_CASE == EX1_TEST_NO_YIELD
#define EX1_EXTI_PRIORITY ISR_NVIC_PRIO
#define EX1_USE_ISR_YIELD 0
#elif EX1_TEST_CASE == EX1_TEST_LOWER_PRIO
#define EX1_EXTI_PRIORITY (ISR_NVIC_PRIO + 1U)
#define EX1_USE_ISR_YIELD 1
#else
#error "Invalid EX1_TEST_CASE"
#endif

/* PB1 -> EXTI1, PA7 -> xung HIGH khi task phản hồi. */
#define EX1_BUTTON_PORT GPIOB
#define EX1_BUTTON_PIN  ((uint16_t)(1U << EXTI_PIN))
#define EX1_BUTTON_IRQn EXTI1_IRQn
#define EX1_ACK_PORT GPIOA
#define EX1_ACK_PIN  ((uint16_t)(1U << ACK_PIN))
#define EX1_EXTI_SUBPRIORITY 0U

/* Giữ các giá trị này giống nhau giữa các lượt đo. */
#define EX1_ACK_TASK_PRIORITY    3U
#define EX1_ACK_TASK_STACK_WORDS 128U
#define EX1_ACK_PULSE_MS         1U /* Delay theo tick, không phải xung chính xác 1 ms. */
#define EX1_NOTIFY_WAIT_TICKS    portMAX_DELAY

#if EXTI_PIN != 1
#error "Update EX1_BUTTON_IRQn and the IRQ handler for a different EXTI pin"
#endif
#if EX1_ACK_TASK_PRIORITY == 0 || EX1_ACK_TASK_PRIORITY >= configMAX_PRIORITIES
#error "ack_task must have a valid priority above Idle"
#endif
#if EX1_TEST_CASE == EX1_TEST_ASSERT
#if EX1_EXTI_PRIORITY >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
#error "ASSERT test requires an interrupt priority above the FreeRTOS threshold"
#endif
#else
#if EX1_EXTI_PRIORITY < configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY || EX1_EXTI_PRIORITY > configLIBRARY_LOWEST_INTERRUPT_PRIORITY
#error "EXTI priority is outside the permitted FromISR range"
#endif
#endif

#endif /* WEEK3_EX1_EXPERIMENT_CONFIG_H */
