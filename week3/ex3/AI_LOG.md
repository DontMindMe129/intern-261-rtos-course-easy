# AI_LOG — Week 3 / Ex3

`student_id`: 2412108 · AI: ChatGPT/Codex · Tổng hợp: 2026-11-13

Prompt được tóm tắt theo chủ đề. Người học gen CubeMX, flash/debug, gửi lệnh
UART và chạy script trên máy tính, thu ảnh/log/JSON. AI giải thích, viết/sửa
source theo yêu cầu, kiểm tra build, đọc bằng chứng và hỗ trợ report.

## Nội dung hỗ trợ

| Prompt / chủ đề | Hỗ trợ và nội dung áp dụng |
|---|---|
| Cấu hình CubeMX, CMake và FreeRTOS. | Nối Cortex-M3, heap_4, stream buffer và task notifications; kiểm tra USART1 115200 baud, RX IRQ priority 8, PA7 output. Giải thích giới hạn priority ISR gọi API FreeRTOS. |
| Viết từng TODO, khi nào bật RX? | Tạo stream buffer trước scheduler, chỉ arm RX từ parser sau scheduler start. Callback nhận một byte, SendFromISR, đếm drop, re-arm và yield. Có callback xử lý lỗi UART và bộ đếm lỗi rearm. |
| Parser hoạt động thế nào, discarding là gì? | Ghép byte tới CR, nhận tối đa 31 byte nội dung, bỏ toàn bộ lệnh quá dài tới CR; hỗ trợ CRLF. Kiểm tra số thập phân và tràn uint32 trước phép nhân/cộng. Giữ TODO theo yêu cầu. |
| P/S/T tác động tới demo ở đâu? | P đổi biến period dùng ở vòng delay tiếp theo; S gọi vTaskPrioritySet; T suspend/resume task. Tạo demo handle trước parser, demo toggle PA7. Giải thích scope, delay và trạng thái task. |
| Kiểm thử nhanh cần parser, UART và script gì? | Thêm số thứ tự #sequence, echo OK đầy đủ và truy vấn ? trả ST. Thêm queue TX 8 phần tử, 160 byte/phần tử, enqueue không chờ; TX task riêng tránh block parser. Thêm script pyserial gửi/đọc UART thật và lưu JSON; thêm test parser C bằng host stubs. |
| Tại sao cần bộ đếm và file JSON? | Phân biệt byte RX, stream drop, UART/rearm error, lệnh hợp lệ/sai/quá dài và TX drop/error. Script tổng hợp phản hồi thật, so số thứ tự/nội dung, lấy delta trước/sau; không mô phỏng UART trong phép đo board. |
| Giữ config test đơn giản. | experiment_config.h chọn buffer 128 hoặc 256 byte, giữ nguyên flow ISR/parser và queue TX khi so sánh. Không sửa params.h tự sinh. |
| Prediction và commit trước flash. | Giải thích thời gian byte 86,8 µs, khả năng chứa lệnh, nguyên nhân overflow và ACK nghẽn TX. Prediction commit 6a08308. |
| Test P/S/T, xem priority bằng gì? | Đối chiếu ảnh P100 làm khoảng toggle đổi 200 → 100 ms; S2 đổi uxPriority 1 → 2; T0/T1 làm PA7 dừng rồi chạy lại. Dùng Watch TCB có sẵn, không thêm biến debug sau khi người học làm rõ ý. |
| Kiểm lệnh sai/quá dài và nhận lại. | Kiểm ảnh P0, S5, T2, số tràn trả ERR command, chuỗi quá dài trả ERR length, sau đó P100 trả OK. Lưu ảnh tên mô tả, giữ ảnh gốc và kiểm tra SHA-256. |
| Chạy script ở terminal nào? | Hướng dẫn dùng PowerShell thay vì terminal gdb-server, đóng COM trong Hercules, để MCU chạy tự do. Kiểm JSON lượt 100 lệnh nghỉ 20 ms: 100 ACK, không lỗi. |
| Burst thiếu phản hồi có phải mất RX? | Buffer 128: parser đếm 1000 lệnh, 545 ACK, txdrop 455. JSON hiện tại buffer 256: parser đếm 1000 lệnh, 549 ACK, txdrop 451. Cả hai nhận đủ số byte dự kiến và không có lỗi RX/UART. Phân biệt thiếu ACK với mất lệnh RX; không xác nhận độc lập nội dung các lệnh thiếu ACK. |
| Vì sao TX queue đầy dù RX xử lý kịp? | ACK có thêm OK, khoảng trắng và CRLF nên dài hơn lệnh RX ở cùng baud; phản hồi tích lại rồi bị bỏ khi queue đầy. Giải thích 160 byte là kích thước phần tử lưu/copy, không phải số byte thực sự truyền mỗi ACK. |
| Sau burst có nhận tiếp không? | Đối chiếu ảnh P100 trả OK và ? trả ST; uart/rearm đều 0. Chỉ xác nhận tiếp tục nhận sau burst, không gọi là đã kiểm chứng recovery sau lỗi UART thực tế. |
| Điền report từng mục, ngắn gọn. | Điền thời gian byte, số lệnh chứa được, chèn ảnh P/S/T, bảng gửi nhanh và đối chiếu prediction. Giải thích queue vẫn dùng được, stream buffer phù hợp một writer/một reader và có thể đọc theo khối; source hiện đọc từng byte. |
| Câu 6 nói về tốc độ tối đa thế nào? | Làm rõ buffer không tăng baud hoặc tốc độ xử lý dài hạn, chỉ hấp thụ backlog tạm thời. interval=0 bỏ sleep của script nhưng chưa xác nhận UART trên dây liên tục. Hai buffer đều không mất RX ở tải đã thử, chưa định lượng mức tăng tốc độ tối đa. Người học chọn giữ câu trả lời report hiện tại sau khi được lưu ý giới hạn này. |
| Rà soát, ignore và commit. | Build, 4 test offline và check_api.sh PASS. Giữ source/script/report/ảnh/JSON; ghi chú nội bộ, hướng dẫn measurement và cache được ignore. Không sửa lại câu trả lời report trong lần rà soát cuối. |

## Bằng chứng và giới hạn

- [100 lệnh có khoảng nghỉ](measurements/05_paced_128/result_01.json),
  [burst 128 byte](measurements/06_burst_128/result_01.json),
  [burst 256 byte](measurements/07_burst_256/result_01.json).
- JSON 256 byte hiện tại thay lượt cũ 556 ACK/444 txdrop bằng 549 ACK/451
  txdrop; report dùng số liệu hiện tại, không trộn hai lượt.
- Config hiện chọn EX3_TEST_DOUBLE_BUFFER (256 byte), khác profile gốc 128.
- Chưa đo thời gian thực thi parser mỗi byte, tốc độ tối đa trên dây UART,
  hoặc kích hoạt lỗi UART để kiểm recovery. Build/test host không thay
  kiểm thử scheduling và UART trên board.
- Mất ACK không tự chứng minh mất RX. Ngược lại, parser đếm lệnh hợp lệ
  cũng không xác nhận độc lập nội dung từng lệnh không có echo về host.
