#ifndef PARSER_TASK_H
#define PARSER_TASK_H

#include "FreeRTOS.h"
#include "task.h"

/* Set by xTaskCreate before the scheduler starts. */
extern TaskHandle_t demo_task_handle;

void parser_task(void *argument);
void demo_task(void *argument);

#endif /* PARSER_TASK_H */
