# AI_LOG — Week 2 / Ex4

`student_id`: 2412108 · AI: ChatGPT/Codex · Tổng hợp: 2026-10-25

Các prompt được tóm tắt theo chủ đề, không chép nguyên văn. Người học cấu hình
CubeMX, flash/debug và thu log/ảnh trên board. AI giải thích, viết/sửa source
theo yêu cầu, hỗ trợ prediction/report và đối chiếu dữ liệu; không trực tiếp
điều khiển board.

## Nội dung hỗ trợ

| Prompt / chủ đề | Hỗ trợ và nội dung áp dụng |
|---|---|
| CubeMX cần cấu hình gì, kiểm tra code đã gen. | Kiểm tra CPU 72 MHz, HAL tick TIM4, UART1 115200/8N1 polling, PA0/PA1 làm mốc GPIO tùy chọn; thêm CMake, FreeRTOSConfig và hooks theo bài trước. |
| Giải thích event group và notification, tạo note. | Giải thích bit/mask, wait-all, clear-on-exit, timeout; notification nằm trong TCB, bộ đếm Give/Take và giới hạn một task nhận cho mỗi lần gửi. Note giữ local. |
| ListItem, TCB, handle và object khác nhau thế nào? | Minh họa RAM/heap, nút nằm trong TCB, danh sách liên kết có nút mốc, owner/container, Ready/delayed/event list; notification không cần object có danh sách chờ riêng. |
| Viết source từng bước, giữ TODO và trình bày gọn. | Tạo event group, sync priority 2 và producer priority 1, stack 256 word/task. Producer delay ngẫu nhiên 2–4 ms; sync chờ đủ bit, in thời điểm/chu kỳ UART. Dùng TaskHandle_t trực tiếp cho sync. |
| Measurement hoạt động ra sao? | Thêm DWT t0 trước báo hiệu, t1 khi hàm chờ trả về; chỉ lấy mẫu đánh thức hợp lệ, chốt TB/max sau 200 mẫu. Phân biệt kind chờ với kind của mốc đo, loại timeout và tín hiệu đến trước. |
| RAM event group và heap notification là gì? | Tính cấu trúc từ kernel và đo qua StaticEventGroup_t; đo heap trước/sau tạo. Phân biệt 28 byte object với 40 byte heap gồm header/căn chỉnh; notification chỉ kiểm tra heap quanh lần gửi đầu. |
| Tạo prediction nháp, giữ nguyên câu hỏi rồi chép/commit. | Dự đoán RAM, latency, timeout và giới hạn nhiều task nhận; giữ câu hỏi gốc, trả lời bên dưới. Commit riêng PREDICTION.md: a45ec53 trước các lượt đo đã lưu. |
| Kiểm tra config 1. | Đọc ảnh: 200 mẫu event, TB/max 1158 cycles ≈ 16,083 µs; cấu trúc 28 byte, heap 40 byte. Log có đủ mask 0x07, không timeout. |
| Config 2 kẹt 5 mẫu notification, xử lý thế nào? | Notification thường đến khi sync chờ event nên lần Take sau trả về ngay. Đổi riêng ID 0 sang delay 10 ms; config 2 chỉ yêu cầu 200 mẫu notification. Kết quả TB/max 975 cycles ≈ 13,542 µs, heap thêm 0 byte. Giữ ảnh lượt cũ để đối chiếu. |
| Build báo thiếu stm32f1xx_hal_uart.c. | Khôi phục driver từ gói STM32CubeF1 V1.8.7 có sẵn trên máy sau khi kiểm tra header khớp, rồi build lại thành công. |
| Đọc log UART như thế nào? | Giải thích t là thời điểm, period là khoảng giữa hai lần SYNC thành công, bits là giá trị trả về trước clear, notify là bộ đếm Take nhận được. Config 2 có chu kỳ 14 ms vì delay 10 ms cộng thời gian sync/UART trước khi producer tiếp tục. |
| Config 3 kiểm tra gì? | Làm ID 2 delay 160 ms với timeout 80 ms. Đối chiếu 40 TIMEOUT trả 0x03 và 40 SYNC trả 0x07; giải thích bit được giữ khi timeout chưa đủ điều kiện. |
| Config 4 và câu hỏi thêm producer. | Đối chiếu 500 period mỗi cấu hình: 3 producer 6,968 ms, 4 producer 7,000 ms, đều 6–8 ms. Sửa hướng dẫn: config 4 không cần chờ latency_done vì nguồn set cuối có thể không thuộc ID 1/2 được lấy mẫu. |
| Câu 3 đã kiểm chứng nhiều task nhận chưa? | Bổ sung config 5: A/B priority 2 cùng chờ bit; controller priority 1 set bit một lần, sau đó chỉ notify A. Watch/log cho event_A/B=1, notify_A=1, notify_B=0, timeout_B=1, passed=done=1. Một lượt thử, kết quả UART in lại mỗi giây. |
| Lưu ảnh, tạo report nháp rồi chép bản chính. | Chép ảnh gốc vào đúng thư mục measurement; liên kết ảnh/log trong report. Giữ câu hỏi, trả lời ngắn; kiểm tra số liệu và chép draft sang REPORT.md. Note, draft và MEASUREMENT.md được gitignore theo yêu cầu. |

## Kết quả và giới hạn

- Config 1–5 đã build thành công; check_api PASS với 21 file app/Core.
  Build là kiểm tra biên dịch; kết quả phần cứng dựa trên log/ảnh người học lưu.
- Event và notification được so từ hai cấu hình, không phải hai phép đo đồng
  thời. Config 2 dùng ID 0 delay 10 ms; đo notification trước rồi chờ event.
  Hai lần chờ có timeout riêng, không phải một deadline chung 80 ms.
- Latency gồm đường báo hiệu, chuyển task và overhead helper; không phải thời
  gian chuyển context thuần. GPIO chỉ là cách kiểm chứng tùy chọn, không có
  capture logic analyzer cho các kết quả Ex4 này.
- So chu kỳ dùng 500 giá trị period đầu sau dòng period=0, bỏ các dòng cuối bị
  dở. Chu kỳ còn chịu UART polling, tick 1 ms và instrumentation; chênh lệch
  0,032 ms không phải chi phí riêng của producer thứ tư.
- Nhận định ở câu 6 được bổ sung sau khi xem dữ liệu config 4; report ghi rõ,
  không coi là dự đoán đã có trước lượt đo.
- Config 5 kiểm chứng một lần set bit báo cho nhiều task, còn một notification
  gửi cho một task cụ thể. Không kết luận notification không thể mô phỏng các
  chức năng khác nếu bổ sung nhiều lần gửi hoặc thiết kế lại.
- Source hiện chọn EX4_MULTI_WAITERS (config 5). Để tái tạo các lượt khác, đổi
  EX4_TEST_CASE và dùng Debug; build Test1–4 có macro override riêng.
