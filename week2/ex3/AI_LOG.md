# AI_LOG — Week 2 / Ex3

`student_id`: 2412108 · AI: ChatGPT/Codex · Tổng hợp: 2026-10-23

Các prompt được tóm tắt và gom theo chủ đề, không phải bản chép nguyên văn.
Người học tự cấu hình CubeMX, nối logic analyzer, build/flash và thu dữ liệu
trên board. AI giải thích, viết/sửa source theo yêu cầu, đọc capture/CSV/ảnh,
hỗ trợ prediction, report và thao tác Git.

## Nội dung hỗ trợ

| Prompt / chủ đề | Hỗ trợ và nội dung áp dụng |
|---|---|
| Phân tích Ex3 và cần config gì? | Đọc README và tham số; giải thích priority inversion/inheritance, chọn ba GPIO và cấu hình CPU 72 MHz, TIM4 làm HAL tick, kernel riêng. |
| Có bắt buộc đặt nhãn TRACE_LOW/MID/HIGH không? | Tên nhãn không bắt buộc; dùng trực tiếp PA0/PA1/PA2. Ba kênh cần để đánh dấu các mốc trên logic analyzer. |
| Kiểm tra CubeMX, cập nhật CMake/config như bài trước. | Kiểm tra clock, GPIO, handler; thêm kernel Cortex-M3/heap_4, heap 8 KiB, mutex, API đọc priority, tick 1 kHz, preemption bật/time slicing tắt. |
| Tạo note ngắn và thêm hooks.c. | Tạo note local; hook malloc/stack dừng khi lỗi, thêm CMake và bật kiểm tra stack mức 2. |
| Binary semaphore là gì, khác mutex thế nào? | Giải thích token, ownership, Take/Give, trạng thái ban đầu, API ISR và priority inheritance. Binary tạo rỗng nên Give lần đầu; mutex tạo sẵn có thể Take. |
| Các hàm tạo báo lỗi khi nào, continue trong Take làm gì? | Giải thích NULL do thiếu heap, hook malloc, assert tham số; Take có timeout khác timeout 0 và chờ vô hạn. |
| Viết resource_init, tạo task rồi viết từng task. | Giữ TODO, điền hai mode; Low/Mid/High prio 1/2/3, stack 256 word/task. Nối app_main trong main và kiểm tra kết quả tạo/cấp phát. |
| Vì sao busy-work phức tạp hơn Week1/Ex2? | Phân biệt đợi thời gian DWT trôi qua với lượng công việc CPU. Chọn vòng lặp cố định, hiệu chuẩn 10000 vòng bằng DWT để ước lượng số vòng theo ms; preemption không làm vòng lặp tự hoàn tất. |
| Flow GPIO ra sao, High có chạy ngay sau Low Give không? | PA0 báo Low giữ tài nguyên, PA1 báo đoạn busy Mid, PA2 HIGH trước Take và LOW khi lấy được. High priority cao được đánh thức có thể preempt ngay khi Give; tiếp tục từ Take đang chờ. |
| Có cần SPI/I2C hoặc kiểm tra tranh chấp RAM thật không? | Phân biệt tài nguyên giả lập với giao tiếp phần cứng; người học chọn mô phỏng thời gian giữ tài nguyên, không thêm buffer/checksum. |
| Tại sao chọn delay/chu kỳ; có lặp lại inversion không? | Ban đầu Low nghỉ 1 tick sau mỗi lượt; theo yêu cầu, đổi Low sang chu kỳ 100 ms cùng Mid. High giữ chu kỳ 50 ms, offset 1 tick; Mid offset 2 tick. Các lượt High xen giữa có thể lấy tài nguyên ngay. |
| Mutex có cần bật inheritance bằng lệnh riêng không? | Kernel tự nâng Low khi High block chờ mutex; configUSE_MUTEXES bật. Không gọi vTaskPrioritySet để giả lập inheritance. |
| Deadline tính từ đâu, câu 5 phân biệt điều gì? | Deadline tham số là 25 ms; chọn xét từ yêu cầu đến hoàn tất và ghi rõ PA2 chỉ đo chờ. Phân biệt Low còn việc với Low bị Mid cản trả tài nguyên. |
| Tạo draft và kiểm tra prediction rồi commit. | Dự đoán lượt tranh chấp: binary khoảng 39 ms, mutex 19 ms; giải thích Mid chen 20 ms. Người học chỉnh bản chính, AI commit riêng e425000 trước khi flash. |
| Tạo thư mục measurement, hướng dẫn flash và đặt cursor. | Tạo binary_20ms/mutex_20ms/binary_40ms; giữ ảnh/capture/CSV. Hướng dẫn ba kênh chung GND, đo hai cạnh PA2 thay vì cạnh PA0. |
| Kiểm tra binary 20 ms và mutex 20 ms. | Đọc ảnh và CSV 5 triệu mẫu ở 1 MHz; mỗi lượt có 50 xung tranh chấp. Binary 39149–39150 µs; mutex 19104–19106 µs. Mid không chen vào Low ở mode mutex. |
| Vì sao binary 40 ms có xung High rất ngắn phía sau? | Thời gian chờ vượt chu kỳ 50 ms; vTaskDelayUntil không block tại mốc đã lỡ, High chạy bù và lấy tài nguyên ngay nên xung ngắn khoảng 4 µs. |
| Kiểm tra binary 40 ms. | High chờ 59176–59178 µs, tăng khoảng 20,03 ms so với Mid 20 ms. Có 50 xung tranh chấp, vẫn không đạt deadline. |
| Tạo report nháp, sửa theo cách trình bày của người học. | Điền sáu mục, liên kết ảnh/capture, mốc t0/t1, giải thích inheritance và sai lệch; bổ sung dự đoán từ dữ liệu 39,150 + 20 = 59,150 ms rồi so với đo lại 59,178 ms. |
| Có bỏ busy 1 ms của High không, overhead là gì? | Giải thích phải đo lại nếu đổi logic; giữ nguyên source đã đo. Phân biệt overhead với sai số hiệu chuẩn/tick, không quy toàn bộ sai lệch cho overhead. |
| Chép y draft sang report chính, hoàn tất AI_LOG. | Chép nguyên nội dung theo yêu cầu và kiểm tra hai file khớp; note/draft được gitignore. Tổng hợp log hỗ trợ và kiểm tra trước commit. |

## Kết quả và giới hạn

- Source đã build ở binary và mutex; dùng API native FreeRTOS. Ảnh và CSV do
  người học thu trên board; AI không trực tiếp điều khiển board.
- Thời gian block trong report là yêu cầu đến Take thành công, gồm overhead nhỏ.
  Thời gian hoàn tất mutex khoảng 20,1 ms được ước lượng thêm 1 ms busy của High,
  chưa có cạnh GPIO đo riêng lúc hoàn tất.
- PA0 HIGH không có nghĩa Low đang chạy CPU: pin giữ mức khi Low bị preempt.
  Busy-work được hiệu chuẩn gần đúng; ngắt và timing phần cứng tạo sai lệch nhỏ.
- Chỉ đổi RESOURCE_MODE giữa binary/mutex, và MID_WORK_MS khi thử tăng công việc.
  Bộ tham số gốc: Low 20, Mid 20, High period 50, deadline 25 (ms).
- Source hiện để RESOURCE_MODE = 0; params.h local đang MID_WORK_MS = 40 sau lượt
  đo cuối. params.h được gen theo student_id và gitignore như quy ước repository;
  đặt Mid về 20 để tái tạo hai lượt cơ sở. Các build có -DRESOURCE_MODE override
  lựa chọn trong header, nên cần kiểm tra build đang dùng khi flash.
- Note và hai draft giữ local; report chính, firmware và bằng chứng đo lưu Git.
