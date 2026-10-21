# Dự đoán — Week 2 / Ex1

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412108

1. Phép tính kích thước queue tối thiểu (dùng `SAMPLE_PERIOD_MS`, thời gian xử lý
   ước lượng của `filter_task` và `uart_task` ở `UART_BAUD`):

   **Số liệu bài cho và cấu hình đang dùng**

   | Đại lượng | Giá trị | Nguồn |
   |---|---:|---|
   | Chu kỳ lấy mẫu `SAMPLE_PERIOD_MS` | 4 ms | `app/params.h` |
   | Độ dài mỗi queue `QUEUE_LENGTH` | 6 phần tử | `app/params.h` |
   | Cửa sổ lọc `FILTER_WINDOW` | 16 mẫu | `app/params.h` |
   | Tốc độ UART `UART_BAUD` | 115200 baud | `app/params.h` |
   | Khung truyền UART | 8N1 | CubeMX / `main.c` |
   | Tần số SCL của I2C1 | 400000 Hz | CubeMX / `main.c` |

   **Thời gian truyền UART**

   Dòng dữ liệu có dạng `seq,ax,ay,az\r\n`. Độ dài lớn nhất theo kiểu dữ liệu:
   `seq` tối đa 10 ký tự; mỗi trục tối đa 6 ký tự kể cả dấu âm;
   3 dấu phẩy và CRLF chiếm 5 ký tự.

   ```text
   N_byte = 10 + 3 × 6 + 3 + 2 = 33 byte
   ```

   Khung 8N1 dùng 1 bit start + 8 bit dữ liệu + 1 bit stop = 10 bit/byte.
   Không truyền ký tự kết thúc chuỗi '\0'.

   ```text
   Công thức: t_UART = N_byte × N_bit_mỗi_byte / baud
   Thế số:    t_UART = 33 × 10 / 115200
   Kết quả:   t_UART ≈ 0,0028646 s ≈ 2,865 ms
   ```

   **Thời gian đọc I2C**

   Cả hai lựa chọn cảm biến đều dùng địa chỉ thiết bị 7 bit, địa chỉ thanh ghi
   1 byte và đọc liên tiếp 6 byte gia tốc. Giao dịch gồm 1 byte địa chỉ chip + W,
   1 byte địa chỉ thanh ghi, 1 byte địa chỉ chip + R và 6 byte dữ liệu.
   Mỗi byte dùng 8 xung dữ liệu + 1 xung ACK/NACK.

   ```text
   N_xung = (1 + 1 + 1 + 6) × 9 = 81 xung SCL

   Công thức: t_I2C ≈ N_xung / f_SCL
   Thế số:    t_I2C ≈ 81 / 400000
   Kết quả:   t_I2C ≈ 0,0002025 s = 0,2025 ms
   ```

   Các kết quả trên chỉ tính thời gian truyền trên dây: UART dùng dòng dài nhất;
   I2C chưa cộng thời gian START/repeated START/STOP và clock stretching.
   Chưa tính overhead HAL, lọc, định dạng chuỗi, thao tác queue, ngắt và lập lịch.

   **Thời gian còn dư và kích thước queue tối thiểu dự đoán**

   ```text
   Công thức: t_dư = T_lấy_mẫu − t_UART − t_I2C
   Thế số:    t_dư ≈ 4 − 2,865 − 0,2025
   Kết quả:   t_dư ≈ 0,9325 ms
   ```

   [GĐ] Trong cấu hình bình thường, tổng thời gian của các công việc còn lại
   (overhead HAL và giao dịch I2C, lọc, định dạng chuỗi, thao tác queue, ngắt và
   lập lịch) nhỏ hơn 0,9325 ms. Task lấy mẫu chạy đều mỗi 4 ms, không có lỗi
   giao tiếp, khoảng gián đoạn kéo dài hay các mẫu bị dồn đến liên tiếp.

   Với giả định trên, từ lúc bắt đầu đọc một mẫu đến khi xuất xong UART mất
   ít hơn 4 ms. Mẫu trước được xử lý xong trước khi bắt đầu chu kỳ kế tiếp,
   nên tại mỗi queue có tối đa một mẫu đang chờ nhận. Mẫu đã được task nhận
   ra để lọc hoặc truyền không còn chiếm chỗ trong queue. Vì queue phải có
   ít nhất một ô để chuyển mẫu giữa các task, dự đoán tối thiểu là:

   - `queue_raw` = **1 phần tử**.
   - `queue_filtered` = **1 phần tử**.

   So với `QUEUE_LENGTH = 6`, mỗi queue đủ dung lượng theo mô hình dự đoán này
   và có thêm chỗ đệm.

2. Khi hạ `UART_BAUD` xuống 9600 (tạm thời), queue nào sẽ đầy trước? Vì sao?  
   - Khi UART_BAUD = 9600, t_UART = 33 × 10 / 9600 = 34,375ms. Tức lớn hơn nhiều so với chu kỳ lấy mẫu 4ms. 
   - Với thời gian trên queue_filtered sẽ đầy trước bởi vì UART không kịp hoàn thành việc lấy 1 mẫu dữ liệu ra và gửi đi trước khi sensor và filter_task cập nhật các mẫu tiếp theo vào.
3. Với `xQueueSend(..., 0)` (timeout = 0) khi queue đầy: điều gì xảy ra với mẫu đó?  
   - Mẫu mới bị bỏ đi khi queue đầy, các mẫu hiện trong queue được giữ nguyên, đồng thời xQueueSend(..., 0) trả về errQUEUE_FULL.
4. Với `xQueueSend(..., portMAX_DELAY)` khi queue đầy: `sensor_task` sẽ ở trạng thái
   nào? Có nguy cơ gì cho chu kỳ lấy mẫu?  
   - Với xQueueSend(..., portMAX_DELAY), khi queue đầy, sensor_task sẽ block cho đến khi nào queue có chỗ trống để gửi dữ liệu vào. Và ciệc block như thế có nguy cơ dẫn đến trễ chu kỳ lấy mẫu trong lúc chờ gửi => mất mẫu, phản ứng chậm với thay đổi.
5. Độ đầy `queue_raw` theo thời gian ở cấu hình bình thường sẽ trông như thế nào
   (tăng dần / dao động quanh một mức / luôn gần 0)? Vì sao?  

   Trong điều kiện bình thường:
   - Sensor gửi mẫu → độ đầy tăng lên 1.
   - Filter nhận mẫu → độ đầy giảm về 0.
   - Filter xử lý xong → block chờ mẫu tiếp.
