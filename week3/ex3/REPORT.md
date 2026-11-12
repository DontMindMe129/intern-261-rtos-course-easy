# Report — Week 3 / Ex3

`student_id`: 2412108    Commit của PREDICTION.md: `6a08308`

Trạng thái: đã build/link firmware và kiểm thử parser/script offline;
chưa flash, chưa mở COM kiểm thử board và chưa có số đo phần cứng.
Flash và measurement sẽ được thực hiện trong đợt chung sau.

## 1. Thời gian byte & độ sâu stream buffer
Phép tính thời gian 1 byte ở `RX_BAUD`, và số lệnh tối đa chứa được trong
`STREAM_BUFFER_BYTES`.

## 2. Demo 3 loại lệnh
Bằng chứng (log/ảnh) `demo_task` phản ứng đúng với `P`, `S`, `T` — mỗi loại ít
nhất 1 ví dụ có số liệu trước/sau.

## 3. Test gửi nhanh
| | Số lệnh gửi | Số lệnh nhận đúng | Mất/lỗi |
|---|---|---|---|
| Gõ tay bình thường | | | |
| Gửi liên tục (script) | | | |

Nếu có mất byte: giải thích bằng cơ chế (stream buffer đầy, parser bị trễ,
hay nguyên nhân khác) — không chỉ nói "bị mất".

## 4. Vì sao stream buffer, không phải queue
Giải thích bằng lời của bạn.

## 5. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân.

## 6. Câu hỏi đánh đổi
Với `RX_BAUD` của bạn, nếu tăng gấp đôi `STREAM_BUFFER_BYTES`, tốc độ gửi tối
đa không mất byte tăng thêm bao nhiêu (đo thật, không suy luận)?
