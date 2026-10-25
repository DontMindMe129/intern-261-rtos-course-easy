#ifndef EX1_EXPERIMENT_CONFIG_H
#define EX1_EXPERIMENT_CONFIG_H

#include "FreeRTOS.h"
#include "params.h"

/* Cấu hình thử nghiệm cục bộ; không sửa params.h. */
#define EX1_N16 1
#define EX1_S0  2
#define EX1_SF  3
#define EX1_SI  4
#define EX1_N32 5
#define EX1_FI  6
#define EX1_Q1  7
#define EX1_L16 8
#define EX1_STEP16 9
#define EX1_STEP32 10

/* N16/S0/SF/FI/SI: 5 lượt chính. N32: phép thử cửa sổ riêng để làm sau. */
/* Đổi lựa chọn tại đây hoặc qua compiler define EX1_PROFILE. */
#ifndef EX1_PROFILE
#define EX1_PROFILE EX1_N16
#endif

#if EX1_PROFILE < EX1_N16 || EX1_PROFILE > EX1_STEP32
#error "Invalid EX1_PROFILE"
#endif

#if EX1_PROFILE == EX1_N16 || EX1_PROFILE == EX1_N32 || EX1_PROFILE >= EX1_Q1
#define EX1_UART_BAUD UART_BAUD
#else
#define EX1_UART_BAUD 9600U
#endif

#if EX1_PROFILE == EX1_Q1
#define EX1_QUEUE_LENGTH 1U
#else
#define EX1_QUEUE_LENGTH QUEUE_LENGTH
#endif

#if EX1_PROFILE == EX1_L16 || EX1_PROFILE == EX1_N32
#define EX1_LATENCY_ENABLED 1
#else
#define EX1_LATENCY_ENABLED 0
#endif

#if EX1_PROFILE == EX1_STEP16 || EX1_PROFILE == EX1_STEP32
#define EX1_STEP_ENABLED 1
#else
#define EX1_STEP_ENABLED 0
#endif

#define EX1_STEP_AT_SEQ 64U
#define EX1_STEP_LOW 0
#define EX1_STEP_HIGH 1000
#define EX1_STEP_THRESHOLD 500

#if EX1_PROFILE == EX1_N32 || EX1_PROFILE == EX1_STEP32
#define EX1_FILTER_WINDOW (2U * FILTER_WINDOW)
#else
#define EX1_FILTER_WINDOW FILTER_WINDOW
#endif

#if EX1_STEP_ENABLED && EX1_STEP_AT_SEQ < EX1_FILTER_WINDOW
#error "Step test requires a full window of LOW samples first"
#endif

/* Sensor chỉ chờ vô hạn trong SI; các lượt khác gửi không chờ. */
#if EX1_PROFILE == EX1_SI
#define EX1_SENSOR_TIMEOUT_TICKS portMAX_DELAY
#else
#define EX1_SENSOR_TIMEOUT_TICKS 0U
#endif

/* Filter lần lượt: không chờ, chờ 10 ms, hoặc chờ vô hạn. */
#if EX1_PROFILE == EX1_SF
#define EX1_FILTER_TIMEOUT_TICKS pdMS_TO_TICKS(10U)
#elif EX1_PROFILE == EX1_FI || EX1_PROFILE == EX1_SI
#define EX1_FILTER_TIMEOUT_TICKS portMAX_DELAY
#else
#define EX1_FILTER_TIMEOUT_TICKS 0U
#endif

#if (EX1_PROFILE == EX1_FI || EX1_PROFILE == EX1_SI) && INCLUDE_vTaskSuspend != 1
#error "FI/SI require INCLUDE_vTaskSuspend = 1"
#endif

#endif /* EX1_EXPERIMENT_CONFIG_H */
