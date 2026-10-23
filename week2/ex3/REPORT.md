# Report nháp — Week 2 / Ex3

`student_id`: 2412108 · Commit PREDICTION.md: `e425000`

Cấu hình: CPU 72 MHz, tick 1 kHz, preemption bật, time slicing tắt.
Prio của task_low/task_mid/task_high lần lượt là 1/2/3.
task_high có chu kỳ 50 ms, task_low và task_mid có chu kỳ 100 ms.
task_high delay ban đầu 1 tick, task_mid 2 tick để task_low lấy tài nguyên trước.
task_low busy khoảng 20 ms CPU, task_mid 20 ms, task_high 1 ms sau khi Take.
Giữ nguyên logic và lịch chạy giữa hai mode, chỉ đổi loại semaphore/mutex.

## 1. Ảnh đo

Logic analyzer 1 MHz, mỗi capture 5 giây, ba kênh:

- D0/PA0: HIGH khi task_low giữ tài nguyên, kể cả lúc bị preempt.
- D1/PA1: HIGH trong đoạn busy của task_mid.
- D2/PA2: HIGH khi task_high yêu cầu (t0), LOW ngay sau khi Take thành công (t1).

Độ rộng xung D2 là thời gian task_high chờ tài nguyên, có tính overhead GPIO/API.
Sau khi lấy được, task_high còn busy khoảng 1 ms rồi Give.

Binary semaphore, MID_WORK_MS = 20:

![Binary semaphore — High chờ 39,150 ms](<measurements/binary_20ms/Screenshot 2026-10-22 235545.png>)

Mutex, MID_WORK_MS = 20:

![Mutex — High chờ 19,105 ms](<measurements/mutex_20ms/Screenshot 2026-10-23 000104.png>)

Capture gốc `capture.sr` và dữ liệu `data.csv` nằm cùng thư mục với từng ảnh.

## 2. Bảng thời gian block

| Cấu hình | t0 (µs) | t1 (µs) | Block (µs) | Đạt deadline 25 ms? |
|---|---:|---:|---:|---|
| Binary semaphore | 39151 | 78301 | 39150 | Không vì riêng thời gian chờ đã vượt 25 ms |
| Mutex | 52633 | 71738 | 19105 | Có theo ước lượng: chờ + khoảng 1 ms busy ≈ 20,1 ms |


## 3. Vì sao mutex ngắn hơn

Khi task_high block chờ mutex, prio của task_low được tăng từ 1 lên 3 nên
task_mid không preempt được task_low. task_low làm xong rồi Give và trở về
prio 1. task_high lấy mutex, busy 1 ms rồi Give, sau đó task_mid mới chạy.

Binary semaphore không có priority inheritance nên task_mid vẫn chen vào
task_low, làm task_low trả tài nguyên muộn và task_high phải chờ lâu hơn.
Chênh lệch thời gian chờ là:

`39150 − 19105 = 20045 µs ≈ 20 ms`, gần bằng thời gian busy của task_mid.

## 4. task_mid trong lúc inversion

Ở lượt binary đã chọn, task_high chờ từ 39,151 đến 78,301 ms.
task_mid chạy liên tục một đợt từ 40,150 đến 60,187 ms, tức khoảng 20,037 ms
(có thể bị ngắt chen vào). Sau đó task_low làm nốt phần busy còn lại rồi Give.

D0 vẫn HIGH trong lúc task_mid chạy vì task_low vẫn giữ tài nguyên dù bị preempt.
Trong capture 5 giây có 50 đợt task_mid chạy, lặp khoảng mỗi 100 ms.
task_mid không dùng tài nguyên nhưng vẫn gián tiếp làm task_high chờ lâu hơn.

## 5. Lệch so với dự đoán

- Binary: dự đoán 39 ms, đo 39,150 ms, lệch khoảng +0,150 ms.
- Mutex: dự đoán 19 ms, đo 19,105 ms, lệch khoảng +0,105 ms.
- Binary vẫn trễ deadline, còn mutex giảm khoảng 20 ms thời gian chờ, đúng xu hướng dự đoán.

Sai lệch nhỏ có thể do overhead task/GPIO/API, ngắt, vị trí tick và việc hiệu
chuẩn busy chỉ gần đúng; chưa đo tách từng thành phần.

## 6. Tăng MID_WORK_MS từ 20 lên 40 ms

Chỉ đổi MID_WORK_MS từ 20 lên 40, giữ binary semaphore và các cấu hình khác.

Theo số liệu ban đầu, task_high chờ 39,150 ms khi task_mid busy 20 ms.
Nếu task_mid busy thêm 20 ms thì dự đoán task_high cũng chờ thêm khoảng 20 ms:

`39,150 + 20 = 59,150 ms`.

![Binary semaphore — Mid 40 ms](<measurements/binary_40ms/Screenshot 2026-10-23 000437.png>)

| MID_WORK_MS | Thời gian task_high chờ |
|---|---:|
| 20 ms | 39,150 ms |
| 40 ms | 59,178 ms |

Đo lại được 59,178 ms, tăng 20,028 ms so với ban đầu và lệch 0,028 ms so với
dự đoán. Mức tăng gần bằng 20 ms busy thêm của task_mid, khớp với dự đoán.
task_high vẫn không đạt deadline 25 ms vì riêng thời gian chờ đã vượt hạn.
