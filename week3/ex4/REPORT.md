# Report — Week 3 / Ex4

`student_id`: 2412108    Commit của PREDICTION.md: `b1cd5bd`

Trạng thái: đã build/link phần thống kê runtime và snapshot trạng thái task;
giữ nguyên deadlock_demo.c để thu bằng chứng bản gốc. Chưa flash/debug trên board,
chưa có số đo phần cứng. Supervisor/IWDG và bước sửa deadlock chưa triển khai.

## 1. Bảng CPU %
| Task | CPU % (lần đo 1) | CPU % (lần đo 2) |
|---|---|---|
| ... | | |

Giải thích Idle task chiếm bao nhiêu % và vì sao.

## 2. Bằng chứng tìm deadlock
Log/ảnh `uxTaskGetSystemState` **trước khi sửa**: trạng thái từng task, chỉ
rõ `task_alpha`/`task_beta` đang `eBlocked` và đang chờ mutex nào. Giải thích
chính xác thứ tự lấy mutex nào gây ra deadlock (AB-BA).

## 3. Sửa lỗi
Mô tả thay đổi bạn làm để hết deadlock (thứ tự lấy mutex thống nhất). Xác
nhận chạy ổn định sau khi sửa (thời gian chạy thử).

## 4. Watchdog
Phép tính reload IWDG. Log/video board tự reset khi deadlock được tái tạo lại
có chủ đích, kèm thời gian từ lúc treo đến lúc reset — so với
`IWDG_TIMEOUT_MS`.

## 5. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân.

## 6. Câu hỏi đánh đổi
Với `SUPERVISOR_BITS` và chu kỳ check-in của bạn, nếu một task thật sự chạy
chậm (không deadlock, chỉ chậm) trong đúng 1 chu kỳ, supervisor có false-trip
(reset oan) không? Trả lời bằng số đo thời gian thực tế của chu kỳ check-in.
