/* Week 3 / Ex4 -- run-time stats on DWT CYCCNT.
 * Native FreeRTOS API only.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "runtime_stats.h"
#include "supervisor.h"

static uint32_t s_previous_cycles;
static uint64_t s_total_cycles;
extern UART_HandleTypeDef huart1;
volatile uint32_t g_runtime_stats_uart_errors;

static const char *task_state_name(eTaskState state)
{
    switch (state) {
        case eRunning:   return "Running";
        case eReady:     return "Ready";
        case eBlocked:   return "Blocked";
        case eSuspended: return "Suspended";
        case eDeleted:   return "Deleted";
        default:         return "Invalid";
    }
}

static void print_cpu_rows(char *table)//để trình bày dễ đọc hơn
{
    /* Keep the kernel's data; replace its tabs with visible column borders. */
    char *row = table;
    while (*row != '\0') {
        char *end = strchr(row, '\n');
        char *next = end != NULL ? end + 1 : row + strlen(row);
        if (end != NULL) {
            *end = '\0';
        }
        row[strcspn(row, "\r")] = '\0';

        char *cycles = strchr(row, '\t');
        if (cycles == NULL) {
            break;
        }
        *cycles++ = '\0';
        while (*cycles == '\t') {
            ++cycles;
        }
        char *percent = strchr(cycles, '\t');
        if (percent == NULL) {
            break;
        }
        *percent++ = '\0';
        while (*percent == '\t') {
            ++percent;
        }

        char line[112];
        int length = snprintf(line, sizeof(line), "%-16s | %12s | %5s\r\n",
                              row, cycles, percent);
        if (length > 0 && length < (int)sizeof(line)) {
            if (HAL_UART_Transmit(&huart1, (uint8_t *)line,
                                  (uint16_t)length, 100U) != HAL_OK) {
                ++g_runtime_stats_uart_errors;
            }
        }
        row = next;
    }
}

static void print_task_states(void)
{
    /* Four tasks in the first experiment: Alpha, Beta, stats and Idle.
     * Leave room for later additions; a failed snapshot is reported. */
    static TaskStatus_t snapshot[8];
    configRUN_TIME_COUNTER_TYPE total_cycles;
    UBaseType_t count = uxTaskGetSystemState(snapshot,
                                            sizeof(snapshot) / sizeof(snapshot[0]),
                                            &total_cycles);
    const char *title = "\r\nState snapshot\r\n"
                        "Task             | State      | Priority\r\n";
    if (count == 0U) {
        title = "State snapshot failed (task array may be too small)\r\n";
    }
    if (HAL_UART_Transmit(&huart1, (uint8_t *)title,
                          (uint16_t)strlen(title), 100U) != HAL_OK) {
        ++g_runtime_stats_uart_errors;
    }

    for (UBaseType_t i = 0U; i < count; ++i) {
        char line[96];
        int length = snprintf(line, sizeof(line), "%-16s | %-10s | %lu\r\n",
                              snapshot[i].pcTaskName,
                              task_state_name(snapshot[i].eCurrentState),
                              (unsigned long)snapshot[i].uxCurrentPriority);
        if (length > 0 && length < (int)sizeof(line)) {
            if (HAL_UART_Transmit(&huart1, (uint8_t *)line,
                                  (uint16_t)length, 100U) != HAL_OK) {
                ++g_runtime_stats_uart_errors;
            }
        }
    }
}

void runtime_stats_timer_init(void)
{
    /* TODO 1: same DWT enable sequence as week1/ex2 dwt_init() (CoreDebug
     *         DEMCR TRCENA bit, zero DWT->CYCCNT, DWT->CTRL CYCCNTENA bit).
     *         If you already have week1/ex2's dwt_init(), the sequence is
     *         identical -- do not invent a second mechanism. */
    uint32_t saved_primask = __get_PRIMASK();//lấy trạng thái PRIMASK hiện tại
    __set_PRIMASK(1U);//che ngắt
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0U;
    s_previous_cycles = 0U;
    s_total_cycles = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    __set_PRIMASK(saved_primask);
}

uint64_t runtime_stats_timer_value(void)
{
    /* TODO 2: return DWT->CYCCNT. This is portGET_RUN_TIME_COUNTER_VALUE()'s
     *         implementation -- wire it up in FreeRTOSConfig.deltas. */
    /* Extend the raw 32-bit counter. Sample at least once per wrap period:
     * about 59.65 seconds at 72 MHz. Multiple missed wraps are not recoverable.
     * Use a short PRIMASK guard, not FreeRTOS critical APIs, because the
     * kernel also calls this function from its context-switch path.
     * Do not call this stateful reader from NMI/HardFault. */
    
    /*
    Điều kiện là không tràn quá 1 vòng
    Mục tiêu là mở rộng giá trị trả về lên 64bit
    Dùng phép trừ lấy khoảng cách rồi cộng lại vào tổng 64bit
    */
     uint32_t saved_primask = __get_PRIMASK();
    __set_PRIMASK(1U);

    uint32_t current = DWT->CYCCNT;
    uint32_t delta = current - s_previous_cycles;
    s_total_cycles += (uint64_t)delta;
    s_previous_cycles = current;
    uint64_t result = s_total_cycles;

    __set_PRIMASK(saved_primask);
    return result;
}

void runtime_stats_task(void *argument)
{
    (void)argument;
    static char buf[configSTATS_BUFFER_MAX_LENGTH];
    TickType_t last_wake = xTaskGetTickCount();
    TickType_t last_report = last_wake;

    for (;;) {
        /* Check in frequently; keep the original one-second report period. */
        (void)xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(STATS_CHECKIN_MS));
        TickType_t now = xTaskGetTickCount();
        if ((TickType_t)(now - last_report) < pdMS_TO_TICKS(STATS_WINDOW_MS)) {
            supervisor_checkin(SUPERVISOR_BIT_STATS);
            continue;
        }
        last_report = now;

        /* TODO 3: vTaskGetRunTimeStats(buf) and print buf over UART.
         *         Note: this walks every task's TCB -- it is itself not free,
         *         factor that into why STATS_WINDOW_MS should not be too
         *         short. */
        vTaskGetRunTimeStats(buf);

        /* The kernel's formatter casts the absolute count to unsigned int.
         * Percentages use the full configRUN_TIME_COUNTER_TYPE internally.
         * These are cumulative percentages, not one-second deltas. */
        char heading[112];
        int length = snprintf(heading, sizeof(heading),
                              "\r\n[tick=%lu] CPU stats since scheduler start\r\n"
                              "Task             | Cycles_low32 |  CPU%%\r\n",
                              (unsigned long)xTaskGetTickCount());
        if (length > 0 && length < (int)sizeof(heading)) {
            if (HAL_UART_Transmit(&huart1, (uint8_t *)heading, (uint16_t)length, 100U) != HAL_OK) {
                ++g_runtime_stats_uart_errors;
            }
        }

        if (buf[0] != '\0') {
            print_cpu_rows(buf);
        } else {
            const char *message = "No run-time stats available\r\n";
            if (HAL_UART_Transmit(&huart1, (uint8_t *)message,
                                  (uint16_t)strlen(message), 100U) != HAL_OK) {
                ++g_runtime_stats_uart_errors;
            }
        }
        print_task_states();
        char health[144];
        int health_len = snprintf(health, sizeof(health),
            "WD bits=0x%02lx refresh=%lu timeout=%lu max_gap_ticks=%lu\r\n"
            "Checkins alpha=%lu beta=%lu stats=%lu\r\n",
            (unsigned long)g_supervisor_last_bits,
            (unsigned long)g_supervisor_refresh_count,
            (unsigned long)g_supervisor_timeout_count,
            (unsigned long)g_supervisor_max_refresh_gap_ticks,
            (unsigned long)g_alpha_checkins, (unsigned long)g_beta_checkins,
            (unsigned long)g_stats_checkins);
        if (health_len > 0 && health_len < (int)sizeof(health)) {
            if (HAL_UART_Transmit(&huart1, (uint8_t *)health,
                                  (uint16_t)health_len, 100U) != HAL_OK) {
                ++g_runtime_stats_uart_errors;
            }
        }
        /* Only report progress after this iteration's UART work finishes. */
        supervisor_checkin(SUPERVISOR_BIT_STATS);
    }
}
