#ifndef WEEK3_EX2_EXPERIMENT_CONFIG_H
#define WEEK3_EX2_EXPERIMENT_CONFIG_H

#include "FreeRTOS.h"
#include "params.h"

/* Chỉ đổi lựa chọn test; không sửa params.h do tool sinh. */
#define EX2_TEST_NORMAL        1 /* Debounce 15 ms, long-press 600 ms. */
#define EX2_TEST_BLOCK_50MS    2 /* Cố ý delay trong callback debounce. */
#define EX2_TEST_HALF_DEBOUNCE 3 /* 15/2 = 7.5 ms, làm tròn lên 8 ms. */

#ifndef EX2_TEST_CASE
#define EX2_TEST_CASE EX2_TEST_NORMAL
#endif

#if EX2_TEST_CASE == EX2_TEST_NORMAL
#define EX2_DEBOUNCE_MS DEBOUNCE_MS
#define EX2_CALLBACK_BLOCK_MS 0U
#elif EX2_TEST_CASE == EX2_TEST_BLOCK_50MS
#define EX2_DEBOUNCE_MS DEBOUNCE_MS
#define EX2_CALLBACK_BLOCK_MS 50U
#elif EX2_TEST_CASE == EX2_TEST_HALF_DEBOUNCE
#define EX2_DEBOUNCE_MS ((DEBOUNCE_MS + 1U) / 2U)
#define EX2_CALLBACK_BLOCK_MS 0U
#else
#error "Invalid EX2_TEST_CASE"
#endif

#define EX2_LONGPRESS_MS LONGPRESS_MS

/* Không tự tạo sự kiện thử: nhấn/nhả nút thật như chế độ bình thường.
 * Block 50 ms không nhất thiết làm HELD trễ nếu không chồng lên hạn timer.
 * Bản nộp cuối chọn EX2_TEST_NORMAL. */

#endif /* WEEK3_EX2_EXPERIMENT_CONFIG_H */
