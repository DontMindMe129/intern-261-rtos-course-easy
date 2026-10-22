#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f1xx.h"

/* Clock và tick */
// TODO: configCPU_CLOCK_HZ
#define configCPU_CLOCK_HZ 72000000U
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
#define configTOTAL_HEAP_SIZE (12U * 1024U) /* 4 writer + gatekeeper, queue và Idle. */

/* Priority ngắt */
// TODO: configPRIO_BITS
#define configPRIO_BITS 4U
// TODO: configLIBRARY_LOWEST_INTERRUPT_PRIORITY
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15U
// TODO: configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5U
// TODO: configKERNEL_INTERRUPT_PRIORITY
#define configKERNEL_INTERRUPT_PRIORITY (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << 4U)
// TODO: configMAX_SYSCALL_INTERRUPT_PRIORITY
#define configMAX_SYSCALL_INTERRUPT_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << 4U)

/* Hook và kiểm tra lỗi */
// TODO: configUSE_IDLE_HOOK
#define configUSE_IDLE_HOOK 0U
// TODO: configUSE_TICK_HOOK
#define configUSE_TICK_HOOK 0U
// TODO: configCHECK_FOR_STACK_OVERFLOW
#define configCHECK_FOR_STACK_OVERFLOW 2U
// TODO: configUSE_MALLOC_FAILED_HOOK
#define configUSE_MALLOC_FAILED_HOOK 1U
// TODO: configASSERT(x)
#define configASSERT(x)                       \
    do {                                      \
        if (!(x)) {                           \
            __disable_irq();                  \
            while(1){}                        \
        }                                     \
    } while (0)


/* API task */
// TODO: INCLUDE_vTaskDelay
#define INCLUDE_vTaskDelay 1
// TODO: INCLUDE_xTaskDelayUntil
#define INCLUDE_xTaskDelayUntil 1
// TODO: INCLUDE_vTaskSuspend
#define INCLUDE_vTaskSuspend 1

/* Mutex (Ex2) */
#define configUSE_MUTEXES 1

/* Software timer */
// TODO: configUSE_TIMERS

/* Handler: kiểm tra ánh xạ SVC, PendSV, SysTick theo port */
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
