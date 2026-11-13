#ifndef SUPERVISOR_H
#define SUPERVISOR_H

#include "FreeRTOS.h"
#include "event_groups.h"

extern EventGroupHandle_t supervisor_events;

/* Bit assignments -- one per monitored task, up to SUPERVISOR_BITS. */
#define SUPERVISOR_BIT_ALPHA   (1u << 0)
#define SUPERVISOR_BIT_BETA    (1u << 1)
#define SUPERVISOR_BIT_STATS   (1u << 2)

/* Task heartbeat and observation periods, independent of the 1 s print period. */
#define SUPERVISOR_WINDOW_MS 150U
#define STATS_CHECKIN_MS 50U

extern volatile uint32_t g_alpha_checkins;
extern volatile uint32_t g_beta_checkins;
extern volatile uint32_t g_stats_checkins;
extern volatile uint32_t g_supervisor_refresh_count;
extern volatile uint32_t g_supervisor_timeout_count;
extern volatile EventBits_t g_supervisor_last_bits;
extern volatile TickType_t g_supervisor_last_refresh_tick;
extern volatile TickType_t g_supervisor_max_refresh_gap_ticks;

void supervisor_init(void);

/* Call from task_alpha/task_beta's loop once per iteration that makes real
 * progress (i.e. AFTER releasing both mutexes, not before taking them --
 * think about why that placement matters for what this is supposed to
 * detect). */
void supervisor_checkin(EventBits_t bit);

void supervisor_task(void *argument);

#endif /* SUPERVISOR_H */
