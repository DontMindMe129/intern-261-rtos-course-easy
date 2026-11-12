"""Offline checks: real parser C source with HAL/RTOS stubs + host ACK accounting.

Requires host gcc (not arm-none-eabi-gcc). Does not exercise actual scheduling,
UART hardware, the ARM port, or RX interrupt recovery.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

from test_uart_rx import summarize


class ReplyTests(unittest.TestCase):
    def test_corrupted_value_is_not_a_valid_ack(self):
        result = summarize(["P10#1", "P10#2"], ["OK P10#1", "OK P99#2"])
        self.assertEqual(result["confirmed"], 1)
        self.assertEqual(result["missing_sequences"], [2])
        self.assertEqual(result["unexpected_replies"], ["P99#2"])

    def test_duplicate_and_error_are_reported(self):
        result = summarize(["P10#1"], ["OK P10#1", "OK P10#1", "ERR length"])
        self.assertEqual(result["confirmed"], 1)
        self.assertEqual(result["duplicate_replies"], 1)
        self.assertEqual(result["error_replies"], ["ERR length"])

    def test_out_of_order_but_complete(self):
        result = summarize(["P10#1", "P10#2"], ["OK P10#2", "OK P10#1"])
        self.assertEqual(result["confirmed"], 2)
        self.assertEqual(result["missing_sequences"], [])


class FirmwareParserTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("gcc"), "Host gcc required")
    def test_actual_parser_commands_and_frame_recovery(self):
        app = Path(__file__).resolve().parents[1] / "app"
        stubs = {
            "FreeRTOS.h": r'''
#pragma once
#include <stdint.h>
#include <stddef.h>
#define pdFALSE 0
#define pdTRUE 1
#define configMAX_PRIORITIES 5U
#define configTICK_RATE_HZ 1000U
#define portMAX_DELAY UINT32_MAX
#define pdMS_TO_TICKS(x) ((uint32_t)(x))
typedef int BaseType_t;
typedef unsigned UBaseType_t;
typedef uint32_t TickType_t;
''',
            "task.h": r'''
#pragma once
#include "FreeRTOS.h"
typedef void *TaskHandle_t;
void vTaskPrioritySet(TaskHandle_t, UBaseType_t);
void vTaskSuspend(TaskHandle_t);
void vTaskResume(TaskHandle_t);
void vTaskDelay(TickType_t);
''',
            "stream_buffer.h": r'''
#pragma once
#include "FreeRTOS.h"
typedef void *StreamBufferHandle_t;
size_t xStreamBufferReceive(StreamBufferHandle_t, void *, size_t, TickType_t);
''',
            "main.h": r'''
#pragma once
#define GPIOA ((void *)1)
#define GPIO_PIN_7 128U
void HAL_GPIO_TogglePin(void *, unsigned);
''',
        }
        harness = r'''
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include "parser_task.c"
StreamBufferHandle_t rx_stream;
volatile uint32_t g_rx_stream_dropped_bytes, g_rx_bytes, g_uart_rx_errors;
volatile uint32_t g_uart_rearm_errors, g_tx_dropped_lines, g_tx_errors;
static unsigned prio, suspended, resumed, log_count;
static char logs[32][160];
static const char *input;
static size_t cursor;
static jmp_buf finished;
void vTaskPrioritySet(TaskHandle_t h, UBaseType_t p) { assert(h); prio = p; }
void vTaskSuspend(TaskHandle_t h) { assert(h); ++suspended; }
void vTaskResume(TaskHandle_t h) { assert(h); ++resumed; }
void vTaskDelay(TickType_t t) { (void)t; }
void HAL_GPIO_TogglePin(void *p, unsigned pin) { (void)p; (void)pin; }
void uart_rx_start(void) {}
void uart_log_line(const char *s) {
    assert(log_count < 32);
    snprintf(logs[log_count++], 160, "%s", s);
}
size_t xStreamBufferReceive(StreamBufferHandle_t h, void *p, size_t n, TickType_t t) {
    (void)h; (void)n; (void)t;
    if (!input[cursor]) longjmp(finished, 1);
    *(uint8_t *)p = (uint8_t)input[cursor++];
    return 1;
}
int main(void) {
    demo_task_handle = (void *)1;
    assert(apply_command("P100", 4) == pdTRUE && s_demo_period_ms == 100);
    assert(apply_command("P10#42", 6) == pdTRUE && s_demo_period_ms == 10);
    assert(apply_command("S4#0", 4) == pdTRUE && prio == 4);
    assert(apply_command("T0#1", 4) == pdTRUE && suspended == 1);
    assert(apply_command("T1#2", 4) == pdTRUE && resumed == 1);
    const char *bad[] = {"", "P", "P0", "P-1", "P1a0", "P4294967296",
        "S5", "T2", "T00", "X1", "P10#", "P#1", "P1#2#3",
        "P10#4294967296", "P10#a", "P4294967295"};
    for (size_t i=0; i < sizeof(bad)/sizeof(bad[0]); ++i)
        assert(apply_command(bad[i], strlen(bad[i])) == pdFALSE);
    assert(s_demo_period_ms == 10 && prio == 4);

    /* A valid-looking suffix of an oversized frame must never be executed. */
    input = "\r\nP100#1\r\nP123456789012345678901234567890123456789S2\rS2#2\rP0#3\rP50#4\r";
    if (setjmp(finished) == 0) parser_task(NULL);
    assert(g_commands_accepted == 3);
    assert(g_commands_rejected == 1);
    assert(g_commands_oversized == 1);
    assert(s_demo_period_ms == 50 && prio == 2);
    assert(log_count == 6); /* READY, OK, ERR length, OK, ERR command, OK */
    assert(strcmp(logs[1], "OK P100#1\r\n") == 0);
    assert(strcmp(logs[2], "ERR length\r\n") == 0);
    assert(strcmp(logs[3], "OK S2#2\r\n") == 0);
    puts("Actual C parser checks passed");
    return 0;
}
'''
        build_root = app.parent / "firmware" / "build" / "host-tests"
        build_root.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(prefix="parser-", dir=build_root) as folder:
            root = Path(folder)
            assert root.resolve().is_relative_to(build_root.resolve())
            for name, contents in stubs.items():
                (root / name).write_text(contents, encoding="utf-8")
            source = root / "harness.c"
            source.write_text(harness, encoding="utf-8")
            exe = root / "test_parser.exe"
            subprocess.run([shutil.which("gcc"), "-std=c11", "-Wall", "-Wextra",
                            "-Werror", "-I", str(root), "-I", str(app),
                            str(source), "-o", str(exe)], check=True)
            subprocess.run([str(exe)], check=True)


if __name__ == "__main__":
    unittest.main()
