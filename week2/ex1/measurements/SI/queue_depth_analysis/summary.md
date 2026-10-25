# Ex1 — Queue log

Nguồn: dump RAM người dùng cung cấp.

| Queue | Đỉnh quan sát | Lần đầu thấy đầy (ms) |
|---|---:|---:|
| raw | 6 | 64 |
| filtered | 6 | 28 |

- Mẫu thiếu giữa các dòng UART hợp lệ: 0.
- Dòng không hợp lệ: 0; seq trùng: 0; seq lùi/reset: 0.

- Độ đầy lấy mẫu mỗi 2 ms có thể bỏ sót đỉnh ngắn.
- Lần đầu thấy đầy là thời điểm quan sát, không phải cạnh đầy chính xác.
- Seq thiếu chưa phân biệt lỗi firmware với lỗi thu UART; không đếm phần đuôi chưa nhận.
- Seq liên tục không chứng minh task lấy mẫu đúng chu kỳ.
- Chép bộ đếm measurement_errors từ Watch vào report riêng.
