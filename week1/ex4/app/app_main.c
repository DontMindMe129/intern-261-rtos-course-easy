/* Week 1 / Ex4 — entry point. */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"

void ram_report_task(void *argument);

/* TODO 1: static storage for stat_task — TCB and stack.
 *         The stack array type matters; it is not char[]. */

/* Bộ nhớ cấp sẵn cho stat_task */
static StaticTask_t statTaskTCB;
static StackType_t statTaskStack[STATIC_TASK_STACK_WORDS];

/* Handle để đo stack của hai task sau này */
TaskHandle_t dynTaskHandle = NULL;
TaskHandle_t statTaskHandle = NULL;
TaskHandle_t AddTaskHandle = NULL;


static void dyn_hog_work(void *argument)
{
    (void)argument;

    volatile uint8_t hog[HOG_DEPTH_BYTES];

    for (;;) {
        for (size_t i = 0; i < sizeof(hog); i++) {
            hog[i] = 0x5AU;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void shared_work(void *argument)
{
    (void)argument;
    for (;;) {
        /* TODO 2: identical trivial work in both tasks, so that any difference
         *         in high water mark comes from how they were created, not from
         *         what they do. */
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void app_main(void)
{
    /* TODO 3: dyn_task  via xTaskCreate,       stack DYN_TASK_STACK_WORDS
     *         stat_task via xTaskCreateStatic, stack STATIC_TASK_STACK_WORDS
     *         ram_report_task via xTaskCreate
     *
     * Check every return value. xTaskCreateStatic returns a handle, not a
     * BaseType_t — explain the difference in REPORT.md. */
/*
    BaseType_t result = xTaskCreate(
        shared_work,
        "dyn_task",
        DYN_TASK_STACK_WORDS,
        NULL,
        1,
        &dynTaskHandle
    );

    configASSERT(result == pdPASS);
*/


    BaseType_t result = xTaskCreate(
        shared_work,
        "add_task",
        128U,
        NULL,
        1,
        &AddTaskHandle
    );
    configASSERT(result == pdPASS);



    result = xTaskCreate(
        dyn_hog_work,
        "dyn_task",
        DYN_TASK_STACK_WORDS + 60U,
        NULL,
        1,
        &dynTaskHandle
    );
    configASSERT(result == pdPASS);


    statTaskHandle = xTaskCreateStatic(
        shared_work,
        "stat_task",
        STATIC_TASK_STACK_WORDS,
        NULL,
        1,
        statTaskStack,
        &statTaskTCB
    );

    BaseType_t reportResult = xTaskCreate(
        ram_report_task,
        "ram_report",
        256U,
        NULL,
        1,
        NULL
    );

    configASSERT(reportResult == pdPASS);

    configASSERT(statTaskHandle != NULL);

    /* TODO 4 (step 6 of the README, enable only for that run):
     *         declare a HOG_DEPTH_BYTES local array inside dyn_task's work
     *         function and write to every element, so the overflow is real and
     *         not optimised away. Think about what -O2 does to a dead array. */

    vTaskStartScheduler();

    /* Scheduler returned => not enough heap for the idle/timer task. */
    for (;;) {
    }
}


