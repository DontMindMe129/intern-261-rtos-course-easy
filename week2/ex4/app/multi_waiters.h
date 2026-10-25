#ifndef EX4_MULTI_WAITERS_H
#define EX4_MULTI_WAITERS_H

#include <stdint.h>

typedef struct {
    uint32_t event_a;
    uint32_t event_b;
    uint32_t notify_a;
    uint32_t notify_b;
    uint32_t timeout_b;
    uint32_t passed;
    uint32_t done;
} multi_waiters_result_t;

extern volatile multi_waiters_result_t measurement_multi_waiters;

void multi_waiters_init(void);

#endif
