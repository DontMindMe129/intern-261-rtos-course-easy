#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f1xx.h"

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

/* Hook và kiểm tra lỗi: chưa bật hook khi Ex2 chưa có app/hooks.c. */
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
// TODO: INCLUDE_vTaskDelete - init_task tự xóa sau khi bật EXTI.
#define INCLUDE_vTaskDelete 1
// TODO: INCLUDE_vTaskDelay
#define INCLUDE_vTaskDelay 1
// TODO: INCLUDE_xTaskDelayUntil
#define INCLUDE_xTaskDelayUntil 1
// TODO: INCLUDE_vTaskSuspend
#define INCLUDE_vTaskSuspend 1
// TODO: INCLUDE_uxTaskPriorityGet
#define INCLUDE_uxTaskPriorityGet 1
// TODO: INCLUDE_xTaskGetCurrentTaskHandle
#define INCLUDE_xTaskGetCurrentTaskHandle 1

/* Mutex */
#define configUSE_MUTEXES 1

/* Task notification: giữ API sẵn có, Ex2 dùng software timer. */
#define configUSE_EVENT_GROUPS 0
#define configUSE_TASK_NOTIFICATIONS 1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES 1

/* Cho phép vTaskGetInfo() khi kiểm tra thông tin task. */
#define configUSE_TRACE_FACILITY 1

/* Software timer (Ex2): daemon task dùng chung cho mọi callback. */
// TODO: configUSE_TIMERS
#define configUSE_TIMERS 1U
// TODO: configTIMER_TASK_PRIORITY - task priority, số lớn ưu tiên cao.
#define configTIMER_TASK_PRIORITY 3U
// TODO: configTIMER_QUEUE_LENGTH - số lệnh start/reset/stop chờ daemon.
#define configTIMER_QUEUE_LENGTH 10U
// TODO: configTIMER_TASK_STACK_DEPTH - số StackType_t, không phải byte.
#define configTIMER_TASK_STACK_DEPTH 256U

/* Handler của port Cortex-M3. */
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
