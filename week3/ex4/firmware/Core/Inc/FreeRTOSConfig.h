#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f1xx.h"
#include "runtime_stats.h"

/* Clock và tick */
// TODO: configCPU_CLOCK_HZ
#define configCPU_CLOCK_HZ SystemCoreClock
// TODO: configTICK_RATE_HZ
#define configTICK_RATE_HZ 1000U
// TODO: configTICK_TYPE_WIDTH_IN_BITS
#define configTICK_TYPE_WIDTH_IN_BITS TICK_TYPE_WIDTH_32_BITS

/* Scheduler */
// TODO: configUSE_PREEMPTION
#define configUSE_PREEMPTION 1U
// TODO: configUSE_TIME_SLICING
#define configUSE_TIME_SLICING 0U
// TODO: configMAX_PRIORITIES
#define configMAX_PRIORITIES 5U
// TODO: configUSE_PORT_OPTIMISED_TASK_SELECTION
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0U

/* Bộ nhớ */
// TODO: configSUPPORT_DYNAMIC_ALLOCATION
#define configSUPPORT_DYNAMIC_ALLOCATION 1U
// TODO: configSUPPORT_STATIC_ALLOCATION
#define configSUPPORT_STATIC_ALLOCATION 0U
// TODO: configMINIMAL_STACK_SIZE
#define configMINIMAL_STACK_SIZE 128U
// TODO: configTOTAL_HEAP_SIZE
#define configTOTAL_HEAP_SIZE (8U * 1024U)

/* Priority ngắt */
// TODO: configPRIO_BITS
#define configPRIO_BITS 4U
// TODO: configLIBRARY_LOWEST_INTERRUPT_PRIORITY
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15U
// TODO: configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
/*
NVIC priority 0–4 : không được gọi API FreeRTOS
NVIC priority 5–15: được gọi các API có hậu tố FromISR
*/
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5U
// TODO: configKERNEL_INTERRUPT_PRIORITY
#define configKERNEL_INTERRUPT_PRIORITY (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8U - configPRIO_BITS))
// TODO: configMAX_SYSCALL_INTERRUPT_PRIORITY
#define configMAX_SYSCALL_INTERRUPT_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8U - configPRIO_BITS))

/* Hook và kiểm tra lỗi: chưa bật hook khi Ex4 chưa có app/hooks.c. */
// TODO: configUSE_IDLE_HOOK
#define configUSE_IDLE_HOOK 0U
// TODO: configUSE_TICK_HOOK
#define configUSE_TICK_HOOK 0U
// TODO: configCHECK_FOR_STACK_OVERFLOW
#define configCHECK_FOR_STACK_OVERFLOW 0U
// TODO: configUSE_MALLOC_FAILED_HOOK
#define configUSE_MALLOC_FAILED_HOOK 0U
// TODO: configASSERT(x)
#define configASSERT(x)           \
    do {                         \
        if (!(x)) {              \
            __disable_irq();     \
            while (1) { }        \
        }                        \
    } while (0)

/* API task */
// TODO: INCLUDE_vTaskDelete - cho phép task khởi tạo tự xóa nếu cần.
#define INCLUDE_vTaskDelete 1
// TODO: INCLUDE_vTaskDelay
#define INCLUDE_vTaskDelay 1
// TODO: INCLUDE_xTaskDelayUntil
#define INCLUDE_xTaskDelayUntil 1
// TODO: INCLUDE_vTaskSuspend
#define INCLUDE_vTaskSuspend 1
// TODO: INCLUDE_vTaskPrioritySet
#define INCLUDE_vTaskPrioritySet 1
// TODO: INCLUDE_uxTaskPriorityGet
#define INCLUDE_uxTaskPriorityGet 1
// TODO: INCLUDE_xSemaphoreGetMutexHolder - hỗ trợ xem task đang giữ mutex.
#define INCLUDE_xSemaphoreGetMutexHolder 1
// TODO: INCLUDE_xTaskGetCurrentTaskHandle
#define INCLUDE_xTaskGetCurrentTaskHandle 1

/* Mutex */
#define configUSE_MUTEXES 1

/* Event group cho supervisor thu các bit báo sống (Ex4). */
#define configUSE_EVENT_GROUPS 1
#define configUSE_STREAM_BUFFERS 0
#define configUSE_TASK_NOTIFICATIONS 1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES 1

/* Quan sát task: uxTaskGetSystemState(), vTaskGetInfo(). */
#define configUSE_TRACE_FACILITY 1

/* Run-time stats (Ex4): bộ đếm DWT tách biệt với RTOS tick. */
// TODO: configGENERATE_RUN_TIME_STATS
#define configGENERATE_RUN_TIME_STATS 1
// TODO: configUSE_STATS_FORMATTING_FUNCTIONS
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
/* vTaskGetRunTimeStats() assumes this exact output-buffer capacity. */
#define configSTATS_BUFFER_MAX_LENGTH 512U
// TODO: configRUN_TIME_COUNTER_TYPE
#define configRUN_TIME_COUNTER_TYPE uint64_t
// TODO: portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() runtime_stats_timer_init()
// TODO: portGET_RUN_TIME_COUNTER_VALUE()
#define portGET_RUN_TIME_COUNTER_VALUE() runtime_stats_timer_value()

/* Software timer: Ex4 không sử dụng; run-time stats không cần timer daemon. */
// TODO: configUSE_TIMERS
#define configUSE_TIMERS 0U

/* Handler của port Cortex-M3. */
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
