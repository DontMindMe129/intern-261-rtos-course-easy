# Report — Week 3 / Ex2

`student_id`: 2412108    Commit của PREDICTION.md: `0f3e893`

Trạng thái: đã build/link; chưa flash và chưa đo phần cứng. Flash và measurement
sẽ thực hiện trong đợt chung sau. Chế độ thử callback block 50 ms chưa được thêm.

## 1. Nảy phím trước/sau
Ảnh logic analyzer: tín hiệu thô (nhiều cạnh nảy) và tín hiệu "PRESS" sau khi
qua `debounce_timer` (một cạnh sạch).

## 2. Callback bị block — thí nghiệm bước 4
| | Dự đoán | Đo được |
|---|---|---|
| Độ trễ `longpress_callback` khi `debounce_callback` block 50ms | | |

Log timestamp (`xTaskGetTickCount()`) của cả hai callback quanh lúc bị block.

## 3. Giải thích daemon task
Giải thích **bằng cơ chế**, không chỉ hiện tượng: vì sao một callback block sẽ
ảnh hưởng timer khác, liên hệ đến timer command queue và việc có đúng MỘT
daemon task xử lý tất cả software timer.

## 4. Long-press
Thời gian giữ thực tế đo được so với `LONGPRESS_MS`: khớp không?

## 5. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân.

## 6. Câu hỏi đánh đổi
Với `DEBOUNCE_MS` của bạn, nếu giảm xuống còn một nửa thì tín hiệu sau debounce
còn sạch không? Đo lại trên board của bạn, không suy luận suông.
