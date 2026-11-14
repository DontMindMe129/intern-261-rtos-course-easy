# AI_LOG — Week 3 / Ex2

`student_id`: 2412108 · AI: ChatGPT/Codex · Tổng hợp: 2026-11-14

Prompt được tóm tắt theo chủ đề. Người học gen CubeMX, flash/debug, nhấn nút
và thu log/capture/ảnh trên board. AI hỗ trợ giải thích, viết/sửa source,
build, phân tích dữ liệu và tổng hợp report; không trực tiếp flash hoặc đo board.

## Nội dung hỗ trợ

| Prompt / chủ đề | Hỗ trợ và nội dung áp dụng |
|---|---|
| Cấu hình và bắt đầu viết source ở đâu? | Nối CMake và FreeRTOSConfig theo format bài trước. Kiểm tra timer daemon, queue lệnh, stack, priority ISR và init task chỉ bật EXTI sau khi scheduler chạy. Giữ TODO. |
| Debounce và long-press chạy lúc nào? | Tạo debounce one-shot 15 ms, long-press auto-reload 600 ms. Mọi cạnh reset debounce; PB1 LOW start long-press, HIGH stop. Giải thích period so với deadline và timestamp lệnh timer. |
| Callback xác nhận PRESS/HELD như thế nào? | Lưu trạng thái ổn định để tránh PRESS lặp; debounce cập nhật PA7 HIGH khi nhấn, LOW khi nhả. HELD kiểm tra PB1 còn LOW trước log. Giải thích callback và lệnh stop có thể được daemon xử lý theo thời điểm khác nhau. |
| UART polling có luôn làm timer kế tiếp trễ không? | Làm rõ callback chiếm daemon chỉ gây trễ timer khác nếu việc xử lý kéo qua deadline; tick vẫn cập nhật trong lúc callback chạy hoặc Blocked. Không mặc định cộng thời gian polling vào mọi timer sau. |
| Timer service task thức/block và xét lịch thế nào? | Giải thích daemon tìm deadline gần nhất để chờ queue, thức khi có lệnh hoặc hết thời gian chờ; sau khi chạy lại kiểm tra tick/deadline. Deadline quá hạn không làm callback chạy khi daemon vẫn Blocked. |
| Prediction và report. | Kiểm tra prediction về nảy, nhấn nhanh, block callback và long-press; prediction commit 0f3e893. Giữ giới hạn đo, không sửa prediction sau khi đã thu dữ liệu. |
| Chọn test bằng config. | Thêm experiment_config.h: normal 15 ms, block 50 ms, half debounce làm tròn 8 ms; giữ params.h tự sinh. Chuyển test bằng macro và build, không tự chèn cạnh nhấn giả. |
| Vì sao 20 kHz rồi 4 MHz vẫn khó thấy nảy? | Đọc mẫu sigrok, tìm nảy lúc nhả dài 206,5 µs và 68,5 µs trong capture 4 MHz; PA7 không lặp cạnh theo nảy. Hướng dẫn zoom, lưu ảnh cursor và phân biệt kênh D0/PA7, D1/PB1. |
| Đo 15 ms có đúng không? | Phân tích 78 cạnh output, độ trễ 14,01225–15,010 ms; ví dụ cụm nhả có delay 14,71975 ms. Giải thích lượng tử hóa tick và vị trí cursor, không đòi chính xác 15,000 ms thực. |
| TX là gì, đo ngưỡng HELD ra sao? | Hướng dẫn nối thêm PA9/TX vào D2. Decode UART 115200/8N1 từ capture, xác nhận HELD đầu sau cạnh nhấn 598,84275 / 599,21250 / 598,87225 ms; HELD lặp tăng đúng 600 tick. Mốc đo là đầu log, không phải đúng lệnh vào callback. |
| Test callback block 50 ms. | Bật delay cuối debounce callback, decode capture khớp log. HELD đầu vẫn khoảng 598,639–599,394 ms; khoảng HELD 600 tick. Không thấy trễ vì khoảng block sớm hơn deadline; không ghi là đã chứng minh timer khác bị trễ. |
| Giảm debounce còn 8 ms. | Đo 34 cạnh output, delay 7,04675–8,01425 ms; cụm nảy khi nhấn dài 802 µs chỉ tạo một cạnh output. Kết luận sạch ở lượt thử, không khái quát cho mọi dạng nảy. |
| Kiểm log/capture và tên file. | Nhận diện các file Session 1 là sigrok, đổi tên capture_02.sr và kiểm hash trước/sau; giữ ảnh gốc, hash-verify bản sao. Phát hiện capture.sr ở test block/8 ms là bản sao dữ liệu lượt trước, không dùng cho kết luận profile mới. |
| Hoàn thiện Ex2. | Khôi phục profile normal, giữ logging theo source đã đo. Điền report ngắn với log/capture và giới hạn. Ban đầu vẽ SVG từ mẫu thực; sau đó thay bằng ảnh PulseView phóng to do người học chụp theo yêu cầu. Ghi AI_LOG, ignore settings viewer và bản capture trùng, kiểm build/API và liên kết bằng chứng. |
| Trả report về mẫu rồi điền từng phần. | Giữ mục 1, trả mục 2–6 về mẫu theo yêu cầu. Người học tự điền phần giải thích; AI thêm số đo cụ thể vào bảng mục 2, điền mục 4/5 và thêm bảng thời điểm các cạnh làm bằng chứng mục 6. Giữ nội dung người học đã sửa. |
| Kiểm tra trước commit/push. | Đối chiếu liên kết log/ảnh/capture, xác nhận profile normal và không còn vTaskDelay thử trong object debounce. Kiểm build, API; chỉ đưa Ex2 và ignore liên quan vào commit, giữ thay đổi bài khác ngoài commit. |

## Kết quả và giới hạn

- Bản cuối: 15 ms, long-press 600 ms, không biên dịch delay thử 50 ms;
  chưa thu lại lượt board sau bước khôi phục này. UART vẫn polling ngắn
  trong callback theo thiết kế đã đo, có thể chiếm daemon khi truyền.
- Chưa có lượt deadline long-press chồng khoảng callback Blocked; phép thử
  hiện tại chỉ cho thấy không chồng deadline thì không thấy trễ thêm.
- Chưa đo double-click ở debounce lớn, chưa hiệu chuẩn clock và chưa readback
  binary để xác nhận tuyệt đối firmware trên chip.
- Report dùng ảnh PulseView do người học chụp; SVG tạo trước đó đã được
  thay khỏi report. Ảnh cursor gốc và capture vẫn được giữ làm bằng chứng.
- Build/API checks không thay kiểm thử phần cứng. Không dùng file capture
  cũ bị sao chép nhầm vào thư mục test mới để kết luận kết quả test đó.
