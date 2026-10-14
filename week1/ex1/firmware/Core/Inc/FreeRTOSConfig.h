#ifndef FREERTOSCONFIG_H_
#define FREERTOSCONFIG_H_

#include "stm32f1xx.h"

//Tick related
#define configCPU_CLOCK_HZ 8000000U
#define configTICK_RATE_HZ 1000U

//Schedule related
#define configUSE_PREEMPTION 1U
#define configMAX_PRIORITIES 2U
#define configUSE_TIME_SLICING 1U
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0U

//Memory related

#define configMINIMAL_STACK_SIZE 128U //stack cua idle task
#define configTOTAL_HEAP_SIZE (4U * 1024U)
#define configCHECK_FOR_STACK_OVERFLOW 2U //tran stack cua mot task
#define configUSE_MALLOC_FAILED_HOOK   1U //khong du heap de tao task

#define configPRIO_BITS 4U // 0-15
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY ((1U << configPRIO_BITS) - 1)
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5U
//dich sang 4 bit cao
#define configKERNEL_INTERRUPT_PRIORITY (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS)) //mac dinh la thap nhat nen khong set cung duoc 
#define configMAX_SYSCALL_INTERRUPT_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
    
#define configTICK_TYPE_WIDTH_IN_BITS TICK_TYPE_WIDTH_32_BITS
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define INCLUDE_vTaskDelay 1

#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#define configASSERT(x)                       \
    do {                                      \
        if (!(x)) {                           \
            __disable_irq();                  \
            while(1){}                        \
        }                                     \
    } while (0)

#endif