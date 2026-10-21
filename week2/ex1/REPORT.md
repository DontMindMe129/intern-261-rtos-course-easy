# Report — Week 2 / Ex1

`student_id`: 2412108    Commit của PREDICTION.md: c4dd892

## 1. Kích thước queue tối thiểu
| | Dự đoán | Đo được | Khớp? |
|---|---|---|---|
| `queue_raw` |1 | | |
| `queue_filtered` |1 | | |

Nếu lệch: nguyên nhân (thời gian xử lý thực tế khác ước lượng, jitter I2C, ...).

## 2. Đồ thị độ đầy queue theo thời gian
Cấu hình bình thường: `![](queue_fill_normal.png)`
Cấu hình UART chậm (9600 baud): `![](queue_fill_slow_uart.png)`

Giải thích hình dạng đồ thị ở mỗi cấu hình.

## 3. Khi nào queue đầy
Mô tả **chính xác** thời điểm `queue_filtered` bắt đầu đầy khi hạ baud, và số
mẫu bị mất/trễ (đếm bằng số thứ tự mẫu gắn trong payload).

## 4. Hành vi 3 kiểu timeout
| Timeout | Kết quả quan sát khi queue đầy |
|---|---|
| 0 | |
| giá trị hữu hạn | |
| `portMAX_DELAY` | |

## 5. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân vật lý.

## 6. Câu hỏi đánh đổi
Với `SAMPLE_PERIOD_MS` của **bạn**, nếu `FILTER_WINDOW` tăng gấp đôi thì độ trễ
từ cảm biến đến UART tăng thêm bao nhiêu ms? Trả lời bằng số từ dữ liệu của bạn,
rồi đo lại để kiểm chứng.
