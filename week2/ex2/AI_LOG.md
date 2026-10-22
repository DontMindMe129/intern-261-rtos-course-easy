# AI_LOG — Week 2 / Ex2

`student_id`: 2412108 · AI: ChatGPT/Codex · Tổng hợp: 2026-10-22

Các prompt dưới đây được tóm tắt và gom theo chủ đề, không phải bản chép nguyên
văn. Người học tự cấu hình CubeMX, build/flash, chạy board, thu log UART và chụp
Watch. AI giải thích, viết/sửa source theo yêu cầu, kiểm tra dữ liệu và hỗ trợ report/Git.

## Nội dung hỗ trợ

| Prompt / chủ đề | Hỗ trợ và phần áp dụng |
|---|---|
| Phân tích Ex2, tạo note ngắn như Ex1. | Giải thích tranh chấp UART và ba cách bảo vệ: mutex, critical, gatekeeper. |
| HAL báo BUSY thì có thể trộn ký tự không? Làm phần simulate lỗi. | Kiểm tra HAL trong project; gửi từng byte, retry BUSY và chủ động yield để tái hiện trộn. Gửi cả mảng thường trả BUSY cho task chen vào; kiểm tra/gán trạng thái HAL không phải mutex nguyên tử. |
| Điền CMake và config dựa các bài trước. | Link kernel native FreeRTOS, heap_4, port Cortex-M3; nối app_main, hooks; CPU 72 MHz, tick 1 kHz, preemption bật, time slicing tắt. |
| Giải thích writer_task, ID/seq, tham số task và chuỗi C. | Một hàm tạo nhiều task; ID truyền qua uintptr_t/void*, seq riêng trên stack; định dạng header và nội dung A–H, phân biệt NUL với byte UART thực gửi. |
| Giải thích flow mode 0, BUSY, timeout, yield và return. | Phân biệt lỗi truyền với ký tự xen; counter có bảo vệ ngắn; timeout 20 ms của mỗi lời gọi HAL không giới hạn tổng thời gian retry BUSY. |
| Làm và giải thích mode 1. | Mutex khóa cả bản tin; Take/Give đúng đường đi, break để không bỏ quên Give. Task/ngắt vẫn chạy; mọi bên dùng UART phải tuân thủ cùng mutex. |
| Làm và giải thích mode 2; TIM4 priority có tác dụng gì? | Polling cả mảng trong critical; BASEPRI che SysTick/TIM4 và ngăn chuyển task. Nêu nguy cơ mất tick và timeout HAL không tiến triển, không thử gây kẹt phần cứng. |
| Làm mode 3; khi enqueue thì keeper có chạy ngay không? | Queue sao chép bản tin; keeper priority 2 cao hơn writer priority 1, preempt khi được đánh thức. Polling giữ CPU nên queue thường không tích nhiều bản tin trong cấu hình này. |
| Phân tích và kiểm tra từng câu prediction, rồi commit. | Thảo luận thời gian UART, latency, RAM, tăng writer. Người học quyết định giữ dự đoán; commit riêng PREDICTION.md: 92bc960 trước khi flash. |
| Viết measurement đơn giản, bám report. | DWT lấy timestamp, thống kê TB/max của 200 bản tin thành công, đo đoạn critical và chênh lệch heap. Không thêm logger task hoặc bản tin đo ra UART. |
| measurement_now/record là gì; packet khác buf thế nào? | Tách lấy mốc khỏi cập nhật thống kê. Keeper nhận timestamp kèm dữ liệu; queue 44 byte/item khi đo, 40 byte/item khi tắt đo. Giải thích overhead timestamp 16 byte/queue. |
| Số RAM bất thường; khác phép đo Week 1 thế nào? | Phát hiện free heap ban đầu bằng 0 vì heap_4 chưa khởi tạo, dẫn đến trừ unsigned bị tràn. Sửa bằng cấp phát rồi trả một vùng nhỏ trước mốc đo; người học nạp lại, mutex đo được 80 byte. |
| Kiểm tra source báo gạch đỏ. | Build sạch thành công; hướng dẫn chọn đúng CMake folder/configure và cập nhật IntelliSense, không sửa code chỉ để xóa gạch đỏ. |
| Flash thế nào, cần lưu gì, tạo thư mục dữ liệu. | Hướng dẫn terminal 115200/8N1, reset từng lượt, chạy tự do trước Pause; tạo thư mục mode0/mutex/critical/gatekeeper, giữ log và ảnh trong Git theo yêu cầu. |
| Kiểm tra log/Watch 4 và 8 writer; có bất hợp lý cần flash lại không? | Đọc sáu log và sáu ảnh; kiểm tra định dạng, ID/nội dung, seq, count/done và lỗi HAL. Ghi nhận mutex 13408→29234 µs TB; critical 3485→3484; keeper 3511→3511. Tăng heap 8→12 KiB cho 8 writer. |
| Vì sao thứ tự 3012; vì sao các lượt trộn khác nhau? | Đối chiếu kernel: task cùng priority tạo cuối được chọn trước khi scheduler chạy; sau đó duyệt Ready list. Delay/tick làm tập task Ready thay đổi, không yêu cầu lỗi có thứ tự trộn cố định. |
| Thử thêm hai lượt mode 0 và kiểm tra. | Cả ba log đều có header/nội dung trộn ngay nhóm đầu; report ghi 3 lượt thử, 3 lượt thấy lỗi. |
| Tạo draft ngắn, kiểm tra rồi chép sang report chính. | Tổng hợp số đo, liên kết bằng chứng, so sánh dự đoán và 4→8 writer; giữ các chỉnh sửa người học, đưa draft vào gitignore. |
| Critical dài 3,48 ms có bằng độ trễ task không? | Phân biệt thời gian che ngắt, chờ CPU và trễ đánh thức do mất tick. Nêu ước lượng từ thời gian critical chưa thay thế đo trực tiếp tổng độ trễ; người học quyết định giữ cách trình bày hiện tại trong report. |

## Kiểm chứng và giới hạn

- Đã build các mode và kiểm tra chỉ dùng API native FreeRTOS. Dữ liệu thực nghiệm
  lấy từ log/ảnh do người học thu trên board; AI không trực tiếp điều khiển board.
- Latency tính từ đầu hàm gửi đến UART truyền xong; không tính thời gian writer
  chờ CPU trước lời gọi. Vì vậy critical/keeper gần như không đổi khi thêm writer
  không có nghĩa tổng thời gian chờ của task không tăng.
- Mutex gửi từng byte có yield; critical/keeper gửi cả mảng. Kết quả gắn với cách
  triển khai và priority hiện tại, không phải thứ hạng chung cho mọi firmware.
- Thời gian critical đo trực tiếp; tổng độ trễ đánh thức task khác/số tick mất chưa đo.
- Measurement source/hướng dẫn và note/draft giữ local theo gitignore đã thống nhất;
  CMake có thể build khi không có measurement. Log, ảnh và report chính được lưu Git.
