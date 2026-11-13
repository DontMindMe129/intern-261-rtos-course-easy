# AI_LOG — Week 3 / Ex4

`student_id`: 2412108 · AI: ChatGPT/Codex · Tổng hợp: 2026-11-13

Prompt được tóm tắt theo chủ đề. Người học gen CubeMX, flash/debug và thu
log UART, ảnh debugger, ảnh logic analyzer trên board. AI giải thích,
viết/sửa source theo từng yêu cầu, build và đối chiếu bằng chứng người học
cung cấp; không trực tiếp flash hoặc thực hiện phép đo phần cứng.

## Nội dung hỗ trợ

| Prompt / chủ đề | Hỗ trợ và nội dung áp dụng |
|---|---|
| Kiểm tra CubeMX, viết CMake và config theo bài trước. | Nối FreeRTOS Cortex-M3, heap_4, runtime stats, trace, mutex và event group; kiểm tra CPU 72 MHz, tick 1 kHz, HAL timebase TIM4 và UART1. Giữ các mục TODO theo yêu cầu. |
| DWT init và hàm lấy giá trị hoạt động thế nào? | Viết init CYCCNT và mở rộng bộ đếm 32 bit thành tổng cycles 64 bit bằng hiệu unsigned. Giải thích PRIMASK, lưu/khôi phục trạng thái ngắt và giới hạn phải đọc trong mỗi vòng tràn khoảng 59,65 giây ở 72 MHz. |
| Viết task thống kê, giải thích buffer và cách in. | Dùng vTaskGetRunTimeStats và uxTaskGetSystemState; in CPU tích lũy, trạng thái và priority. Sửa tab thành cột dễ đọc; giải thích Cycles_low32, chuỗi rỗng và ký hiệu <1% của kernel. |
| Tạo task và kiểm tra prediction. | Nối app_main, tạo Alpha/Beta/stats, giải thích lịch chạy và deadlock AB-BA. Kiểm tra prediction, commit trước các lượt đo đã thu; prediction commit b1cd5bd. |
| Debug để nhận biết deadlock, Queue_t quản lý gì? | Giải thích mutex object, owner và danh sách chờ. Đối chiếu 4 ảnh: mutex A do Alpha giữ, Beta chờ; mutex B do Beta giữ, Alpha chờ. Đối chiếu log cả hai Blocked và cycles không tăng. |
| Sửa lỗi và kiểm tra log sau sửa. | Thống nhất cả hai task lấy A rồi B, nhả B rồi A. Log test 2 có cycles của cả hai tăng qua 7 mẫu; giải thích Blocked tại một snapshot có thể là delay/chờ mutex tạm thời. |
| Supervisor, IWDG và ba nguồn check-in. | Thu bit Alpha/Beta/stats bằng event group, wait-all 150 ms; chỉ refresh khi đủ 0x07. Chọn prescaler 4, reload 2499 cho timeout danh định 250 ms với LSI 40 kHz. Khởi động IWDG trong supervisor task; thêm HAL IWDG cùng phiên bản qua CMake. |
| Stats có cần check-in 50 ms không? | Tách check-in 50 ms khỏi kỳ in 1000 ms để stats không tự làm thiếu heartbeat trước timeout 250 ms. Đã từng bỏ chu kỳ 50 ms theo yêu cầu, sau đó khôi phục khi nhận ra phép thử reset bị nhiễu do stats chậm. Log của cấu hình đó không dùng để kết luận reset riêng do deadlock. |
| Làm sao bảo đảm đổi source qua lại đúng các test? | Không thêm file chọn test cho Ex4 theo lựa chọn người học. Lưu snapshot source/config/ELF và kiểm tra SHA-256 ở các mốc chuyển test; bản sao local không thay bằng chứng firmware thực tế trên chip. |
| Kiểm tra watchdog bình thường và lúc deadlock. | Test 3: 18 mẫu, refresh tăng, timeout 0, không thấy reset, max refresh gap 108 ms. Test 4 giữ cấu hình giám sát, tái tạo thứ tự mutex đối nghịch; kiểm tra log reset do IWDG. |
| Đo từ deadlock tới reset, có cần NRST không? | Giải thích boot log đọc cờ reset phần cứng, không phải callback tại lúc reset. Thêm PA7 làm mốc xấp xỉ trước khi vòng chờ khép kín; dùng cạnh xuống NRST làm mốc reset. Đối chiếu ảnh 1 MHz: 241,520 ms. Lưu ảnh có tên mô tả và giữ ảnh gốc. |
| Test 5 kiểm tra gì, vì sao đo gần 300 ms? | Ban đầu chèn chậm sau 2 giây; giải thích bit Alpha cũ có thể cho phép refresh sau marker. Người học yêu cầu đơn giản hóa: cho Alpha delay 350 ms ngay vòng đầu sau khi nhả mutex, trước check-in đầu tiên. Bỏ log SLOW có thể tranh chấp UART; đối chiếu bản mới có 61 reset IWDG và ảnh 241,552 ms. |
| Trả source bình thường và điền report từng phần. | Lưu bản fault injection, bỏ slow và marker PA7, giữ thứ tự A → B và watchdog. Điền bảng, ảnh/log deadlock, bằng chứng sau sửa, công thức IWDG, đối chiếu prediction và câu đánh đổi theo từng yêu cầu; rút gọn phần 4. Sửa bảng CPU từ nguồn test 3 nhầm sang test 1 tại tick 1001/7001. |
| Commit report, rà soát Ex4 và ignore file local. | Commit report 9ded32b trước lần sửa bảng gần nhất; kiểm tra build và check_api.sh PASS. Rà soát tính nhất quán số liệu, các liên kết bằng chứng và phần đo còn thiếu. Bỏ qua snapshot và ghi chú nội bộ, giữ log/ảnh có thể đưa vào Git. |

## Kết quả và giới hạn

- Source hiện tại đã bỏ các lỗi cố ý, cả hai task lấy mutex cùng thứ tự;
  bản này build thành công. Chưa thu thêm log trên board sau bước khôi phục cuối.
- Log/ảnh phần cứng do người học cung cấp; các ảnh lưu được đối chiếu bằng
  SHA-256 với file gốc. Không có readback để xác nhận binary trên chip.
- Hai số 241,520 ms và 241,552 ms là thời gian từ marker PA7 đến reset,
  không phải phép đo trực tiếp timeout từ lần refresh IWDG cuối. Marker
  deadlock đặt trước API nên là mốc xấp xỉ; chưa đo riêng tần số LSI.
- Câu 6 đã chứng minh task chậm có thể gây reset, nhưng chưa đo trực tiếp
  khoảng cách giữa các check-in riêng của task. Không coi 350 ms cấu hình,
  trung bình suy từ số lần check-in hay khoảng refresh là số đo thay thế.
- Snapshot, ghi chú và hướng dẫn đo giữ local; report liên kết log UART và
  ảnh làm bằng chứng. Build/check API không thay thế kiểm thử trên board.
