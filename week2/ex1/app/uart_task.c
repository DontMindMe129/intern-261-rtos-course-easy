/* Week 2 / Ex1 — prints filtered samples to USART1 at UART_BAUD.
 * queue_filtered -> UART.
 *
 * This is also the task you deliberately starve in step 7 (temporarily set
 * UART_BAUD-equivalent hardware baud to 9600) to force queue_filtered to fill.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>

#include "main.h"
#include "params.h"
#include "pipeline.h"

extern UART_HandleTypeDef huart1;

/* Theo dõi lỗi bằng debugger. */
volatile uint32_t uart_format_errors = 0;
volatile uint32_t uart_transmit_errors = 0;

void uart_task(void *argument)
{
    (void)argument;

    /* Dòng dài tối đa 33 byte; chừa thêm chỗ cho ký tự kết thúc chuỗi. */
    char line[40];
    configASSERT(queue_filtered != NULL);

    for (;;) {
        filtered_sample_t in;

        /* TODO 1: xQueueReceive(queue_filtered, &in, portMAX_DELAY). */

        xQueueReceive(queue_filtered, &in, portMAX_DELAY);
            
    

        /* TODO 2: format "seq,ax,ay,az\r\n" and HAL_UART_Transmit on USART1.
         *         Transmit time at UART_BAUD is the thing that makes this
         *         task slow relative to sensor_task/filter_task — say so in
         *         REPORT.md with the actual byte time you computed. */
        int length = snprintf(line, sizeof(line), "%lu,%d,%d,%d\r\n",
                              (unsigned long)in.seq,
                              (int)in.ax, (int)in.ay, (int)in.az);

        /* snprintf trả độ dài cần có; không truyền nếu lỗi hoặc bị cắt. */
        if (length < 0 || (size_t)length >= sizeof(line)) {
            uart_format_errors++;
            continue;
        }

        /* Polling, timeout HAL 100 ms; không gửi byte '\0'.
         * 100 ms cho phép thử dòng 33 byte ở 9600 baud (~34,4 ms trên dây).
         * Khi thử baud chậm, đổi baud thực tế trong CubeMX và terminal.
         */
        if (HAL_UART_Transmit(&huart1, (uint8_t *)line,
                              (uint16_t)length, 100U) != HAL_OK) {
            uart_transmit_errors++;
        }

        (void)in;
    }
}
