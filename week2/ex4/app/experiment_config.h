#ifndef EX4_EXPERIMENT_CONFIG_H
#define EX4_EXPERIMENT_CONFIG_H

#include "params.h"

/* Change only EX4_TEST_CASE, then rebuild Debug and reset the MCU. */
#define EX4_EVENT3   1
#define EX4_NOTIFY3  2
#define EX4_TIMEOUT3 3
#define EX4_EVENT4   4
#define EX4_MULTI_WAITERS 5

#ifndef EX4_TEST_CASE
#define EX4_TEST_CASE EX4_MULTI_WAITERS
#endif

#if EX4_TEST_CASE < EX4_EVENT3 || EX4_TEST_CASE > EX4_MULTI_WAITERS
#error "EX4_TEST_CASE must be 1, 2, 3, 4 or 5"
#endif

/* Keep generated params.h unchanged for all experiments. */
#if EX4_TEST_CASE == EX4_EVENT4
#define EX4_PRODUCER_COUNT (EVENT_BIT_COUNT + 1U)
#else
#define EX4_PRODUCER_COUNT EVENT_BIT_COUNT
#endif

#ifndef PRODUCER0_USE_NOTIFY
#define PRODUCER0_USE_NOTIFY (EX4_TEST_CASE == EX4_NOTIFY3)
#endif

/* Let sync finish UART output and block before the next notification. */
#define EX4_NOTIFY_PRODUCER_PERIOD_MS 10U

#define EX4_SLOW_PRODUCER_ID       2U
#define EX4_SLOW_PRODUCER_DELAY_MS (2U * SYNC_TIMEOUT_MS)

#if EX4_PRODUCER_COUNT < 2 || EX4_PRODUCER_COUNT > 24
#error "This experiment requires 2..24 producers with 32-bit ticks"
#endif

#endif
