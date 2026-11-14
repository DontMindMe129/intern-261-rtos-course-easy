# AI_LOG — Week 3 / Ex1

`student_id`: 2412108 · AI: ChatGPT/Codex · Tổng hợp: 2026-11-14

Prompt được tóm tắt theo chủ đề. Người học gen CubeMX, flash/debug, nhấn nút,
thu capture và chụp ảnh trên board. AI giải thích, viết/sửa source theo yêu
cầu, kiểm tra build, phân tích dữ liệu và hỗ trợ report; không trực tiếp
flash hoặc điều khiển phép đo phần cứng.

## Nội dung hỗ trợ

| Prompt / chủ đề | Hỗ trợ và nội dung áp dụng |
|---|---|
| Tạo CMake/config sau khi gen CubeMX. | Nối FreeRTOS Cortex-M3, heap_4, configASSERT, exception handlers theo format bài trước; kiểm tra PB1/EXTI1, PA7 và clock CPU 72 MHz. |
| Viết init, ISR, ack_task, tạo task và callback. | Init GPIO thủ công, EXTI falling pull-up, PA7 mặc định LOW. ISR notify rồi yield; ack_task chờ notification rồi phát xung HIGH. Chỉ bật IRQ khi scheduler chạy và task handle đã hợp lệ. Giữ TODO theo yêu cầu. |
| ISR table, callback HAL và priority. | Giải thích vector EXTI gọi HAL handler rồi callback ứng dụng; phân biệt NVIC priority số nhỏ ưu tiên cao với task priority số lớn ưu tiên cao. Ngưỡng FromISR là 5, hợp lệ 5–15. |
| configASSERT báo lỗi bằng biến gì? | Thêm g_assert_failed, set 1 khi assert sai, che ngắt rồi lặp vô hạn để debug. Giải thích vPortValidateInterruptPriority và so sánh priority đã dịch bit. |
| Chọn testcase và đo latency. | Thêm experiment_config.h: priority 4/assert, priority 8/yield, priority 8/no yield, priority 9/yield. Dùng PB1 xuống → PA7 lên; đo bằng logic analyzer, tránh cạnh nảy khi task đang phát ACK. |
| Prediction số cycles và disassembly đọc thế nào? | Hướng dẫn đọc objdump, theo flow/nhánh và lời gọi hàm; phân biệt số lệnh với cycles. Ước lượng 500 lệnh × 2 cycles ở 72 MHz ≈ 13,9 µs; không gọi đây là số đo DWT. Prediction commit 1509e03. |
| Test assert có đủ bằng chứng chưa? | Đối chiếu ảnh flag 0 → 1 và vị trí dừng tại configASSERT(ucCurrentPriority >= ucMaxSysCallPriority) trong port.c. Lưu 3 ảnh, hash-verify bản sao và giữ ảnh gốc. |
| Đo priority 8 có yield. | Phân tích 13 mẫu nhấn tách biệt: min 14,00, mean 14,1731, max 14,25 µs. Hướng dẫn cursor; lưu ảnh sửa lại hiển thị 14,261297 µs, ưu tiên số liệu mẫu gốc khi thống kê. |
| Bỏ yield tăng latency bao nhiêu? | Build profile no-yield, kiểm 26 mẫu: min 42,50, mean 460,4038, max 1003,25 µs; 24 mẫu vượt budget 80 µs. Chênh mean hai lượt 446,23 µs; giải thích phụ thuộc pha tick, không mặc định cộng đúng 1 ms. |
| Priority 8 → 9 có đổi latency không? | Build profile priority 9/yield, chọn 30 mẫu, loại 15 cạnh sát nhau/ACK còn bận. Min 14,00, mean 14,375, max 18,00 µs. Điển hình gần như không đổi ở tải hiện tại; không suy ra mọi mẫu và mọi tải đều giống nhau. |
| File .deltas và configASSERT điền gì? | Đối chiếu tài liệu với FreeRTOSConfig.h: 4 priority bits, lowest 15, syscall threshold 5; macro thực tế set flag rồi dừng. Người học điền .deltas; làm rõ file này không được compiler dùng trực tiếp. |
| Điền report từng câu, ngắn gọn. | Điền mục assert, bảng latency/ảnh, bảng so prediction; thêm bằng chứng priority 8/9 vào mục câu hỏi đánh đổi (mục 5). Người học tự viết giải thích priority; sửa lỗi gõ và làm rõ số priority nhỏ nghĩa là ưu tiên cao. |
| Rà soát, AI_LOG, commit/push cả report ex2 mới sửa. | Khôi phục profile priority 8 có yield, kiểm source/build/API và đường dẫn bằng chứng. Giữ log/capture/ảnh chính; ignore viewer settings và ảnh cursor cũ. Giữ các chỉnh sửa report ex2 của người học, không đưa source các bài khác vào commit. |

## Kết quả và giới hạn

- Bản cuối chọn EX1_TEST_YIELD, priority EXTI 8, ack_task priority 3, xung
  HIGH rồi delay 1 tick. Chưa thu lại log board sau bước khôi phục cuối.
- Các số đo là cạnh PB1 đến cạnh PA7, gồm HAL/ISR, notify, context switch
  và GPIO write, không phải riêng thời gian thực thi ISR hoặc API.
- Capture 4 MHz có bước mẫu 0,25 µs; chưa hiệu chuẩn clock hoặc readback
  binary trên chip. Thống kê chỉ chọn nhấn tách biệt khi task đã chờ.
- Priority 9 có hai mẫu 16,25 và 18,00 µs; chưa xác định chính xác nguyên
  nhân. Không suy ra chênh mean khoảng 0,20 µs là tác động cố định của priority.
- Có ảnh assert ở profile lỗi; chưa có ảnh riêng g_assert_failed=0 ở profile
  đúng. Capture có ACK lặp bình thường phù hợp việc CPU không bị assert giữ.
- Build/API checks không thay phép đo thực tế; không gọi min/max quan sát
  trên các lượt hữu hạn là worst-case của mọi điều kiện hệ thống.
