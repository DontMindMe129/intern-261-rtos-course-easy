/* Week 3 / Ex4 -- supervisor task: waits for all monitored tasks to check in
 * via an event group every cycle; refreshes IWDG only if they all did.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "main.h"
#include "params.h"
#include "supervisor.h"

EventGroupHandle_t supervisor_events;

#define ALL_BITS ((1u << SUPERVISOR_BITS) - 1u)

#if SUPERVISOR_BITS != 3
#error "This test monitors Alpha, Beta and stats: exactly three sources"
#endif

static IWDG_HandleTypeDef s_iwdg;
volatile uint32_t g_alpha_checkins;
volatile uint32_t g_beta_checkins;
volatile uint32_t g_stats_checkins;
volatile uint32_t g_supervisor_refresh_count;
volatile uint32_t g_supervisor_timeout_count;
volatile EventBits_t g_supervisor_last_bits;
volatile TickType_t g_supervisor_last_refresh_tick;
volatile TickType_t g_supervisor_max_refresh_gap_ticks;

void supervisor_checkin(EventBits_t bit)
{
    if (bit == SUPERVISOR_BIT_ALPHA) {
        ++g_alpha_checkins;
    } else if (bit == SUPERVISOR_BIT_BETA) {
        ++g_beta_checkins;
    } else if (bit == SUPERVISOR_BIT_STATS) {
        ++g_stats_checkins;
    }
    xEventGroupSetBits(supervisor_events, bit);
}

void supervisor_init(void)
{
    /* TODO 1: supervisor_events = xEventGroupCreate(); check non-NULL.
     *
     * TODO 2: HAL IWDG init here (IWDG_TIMEOUT_MS -> prescaler + reload per
     *         RM0008 IWDG chapter; IWDG runs off LSI, ~40 kHz, compute the
     *         reload value and show the computation in REPORT.md). */
    supervisor_events = xEventGroupCreate();
    configASSERT(supervisor_events != NULL);
    /* TODO 2: defer hardware start to supervisor_task so setup before the
     * scheduler does not consume the watchdog budget. */
}

void supervisor_task(void *argument)
{
    (void)argument;

    /* /4 at nominal LSI 40 kHz: 250 ms -> Reload = 2499.
     * Freeze only while halted by a debugger; measurement runs freely. */
    __HAL_DBGMCU_FREEZE_IWDG();
    s_iwdg.Instance = IWDG;
    s_iwdg.Init.Prescaler = IWDG_PRESCALER_4;
    s_iwdg.Init.Reload = ((uint64_t)IWDG_TIMEOUT_MS * LSI_VALUE) / (1000U * 4U) - 1U;
    configASSERT(s_iwdg.Init.Reload <= 4095U);
    HAL_StatusTypeDef status = HAL_IWDG_Init(&s_iwdg);
    configASSERT(status == HAL_OK);
    g_supervisor_last_refresh_tick = xTaskGetTickCount();

    for (;;) {
        /* TODO 3: xEventGroupWaitBits(supervisor_events, ALL_BITS, pdTRUE,
         *         pdTRUE, <timeout>) -- clear-on-exit, wait-for-all. Pick a
         *         timeout tighter than IWDG_TIMEOUT_MS but loose enough that
         *         normal scheduling jitter does not cause a false trip;
         *         justify the number in REPORT.md.
         *
         * TODO 4: if the wait actually collected ALL_BITS (check the return
         *         value against ALL_BITS, do not assume success just because
         *         the call returned), HAL_IWDG_Refresh(&hiwdg). Otherwise do
         *         NOTHING -- that is what lets IWDG_TIMEOUT_MS expire and
         *         reset the board when a task stops checking in. */
        EventBits_t bits = xEventGroupWaitBits(supervisor_events, ALL_BITS,
                                              pdTRUE, pdTRUE,
                                              pdMS_TO_TICKS(SUPERVISOR_WINDOW_MS));
        g_supervisor_last_bits = bits & ALL_BITS;
        if ((bits & ALL_BITS) == ALL_BITS) {
            status = HAL_IWDG_Refresh(&s_iwdg);
            configASSERT(status == HAL_OK);
            TickType_t now = xTaskGetTickCount();
            TickType_t gap = now - g_supervisor_last_refresh_tick;
            if (gap > g_supervisor_max_refresh_gap_ticks) {
                g_supervisor_max_refresh_gap_ticks = gap;
            }
            g_supervisor_last_refresh_tick = now;
            ++g_supervisor_refresh_count;
        } else {
            ++g_supervisor_timeout_count;
            /* Drop partial results: the next window needs fresh check-ins. */
            xEventGroupClearBits(supervisor_events, ALL_BITS);
        }
    }
}
