# Report — Week 2 / Ex2

`student_id`: 2412108 · Commit PREDICTION.md: `92bc960`

Cấu hình: STM32F103, CPU 72 MHz, UART 115200 baud/8N1; bản tin 40 byte,
delay 3 tick sau mỗi lần gửi. Writer priority 1, gatekeeper priority 2;
tick 1 kHz, tắt time slicing. Mode 0/1 chủ động yield sau mỗi byte.

## 1. Lỗi tái hiện — mode 0

Nhóm bản tin đầu tiên trong log:

```text
[[[[TTTT3012    ####0000000000000000000000000000000000000000]]]]    DABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABC
```

Vị trí lỗi: `[[[[TTTT3012…` là header của 4 task bị xen ký tự;
`DABCDABC…` là nội dung 4 bản tin bị trộn thay vì từng chuỗi A/B/C/D riêng.
Nguyên nhân: các task gửi từng byte mà không khóa toàn bộ bản tin, rồi yield.

**Số lượt thử / số lượt thấy lỗi: 3 / 3.** Cả ba đều xuất hiện lỗi ngay nhóm đầu;
kiểu trộn về sau có thể thay đổi theo lịch chạy.

Log gốc: [lượt 1](measurements/mode0/uart_log.log),
[lượt 2](measurements/mode0/uart_log_1.log), [lượt 3](measurements/mode0/uart_log_2.log).

## 2. So sánh ba cách — 4 writer

| Cách | Latency TB (µs) | Max (µs) | Ảnh hưởng task khác | RAM thêm (byte) |
|---|---:|---:|---|---:|
| Mutex | 13408 | 15916 | Task khác vẫn chạy; writer phải chờ mutex | 80 |
| Critical | 3485 | 3487 | Trì hoãn chuyển task và các ngắt bị che | 0 |
| Gatekeeper | 3511 | 3514 | Keeper priority cao giữ CPU trong lúc polling UART | 1384 |

Cả ba log đều không trộn bản tin, không gián đoạn `seq` giữa các bản tin đầy đủ.
Ảnh Watch đều có `count = 200`, `done = 1`, BUSY và lỗi truyền bằng 0.
Dòng cuối dở dang trong một số file phù hợp với việc dừng ghi giữa bản tin.

**Cách đo:** DWT đếm chu kỳ từ lúc vào `uart_guard_send()` đến khi HAL truyền
xong bản tin (TC), riêng Gatekeeper nhận timestamp kèm bản tin qua queue.
Đổi sang µs bằng số chu kỳ chia 72; chốt TB/max sau 200 lần gửi thành công.
Không tính tạo chuỗi, hiển thị terminal hoặc thời gian chờ CPU trước lời gọi.

RAM thêm là free heap trước trừ sau `uart_guard_init()`, gồm overhead cấp phát.
Gatekeeper gồm queue và stack/TCB task riêng; số 1384 byte có **16 byte timestamp
trong queue phục vụ đo**. 

Chứng cứ: [mutex](measurements/mutex/4writers_watch.png),
[critical](measurements/critical/4writers_watch.png),
[gatekeeper](measurements/gatekeeper/4writers_watch.png);
log tương ứng nằm cùng thư mục, tên `4writers_uart_log.log`.

## 3. Vì sao critical section nguy hiểm nhất

Với 4 writer, thời gian bên trong critical: **TB 3483 µs, max 3486 µs**.
Gần thời gian truyền lý thuyết: `40 × 10 / 115200 ≈ 3472 µs`.  

Trong khoảng này, BASEPRI che ngắt priority từ 5 trở lên, gồm SysTick và TIM4
(priority 15). Chuyển task bị trì hoãn, tick có thể bị mất; timeout HAL cũng
không tiến triển bình thường. Ngắt priority 0–4 vẫn có thể chạy.  

Trong một lần critical, task cần chạy có thể phải chờ thêm gần 3,49 ms nếu yêu cầu chạy xuất hiện ngay đầu đoạn này. Đây là ước lượng từ thời gian critical, hiện chưa đo trực tiếp độ trễ đánh thức task.


## 4. Lệch so với dự đoán

- Dự đoán `Critical < Mutex < Gatekeeper` trong khi thực tế ghi nhận **Critical < Gatekeeper < Mutex**.
  Mutex gửi từng byte, chờ khóa và yield còn keeper gửi cả mảng, priority cao nên
  xử lý ngay sau enqueue.
- Mode 0 lỗi ngay nhóm đầu, gatekeeper tốn RAM nhiều hơn mutex và mutex tăng
  latency nhiều nhất khi thêm writer: phù hợp dự đoán.

## 5. Tăng từ 4 lên 8 writer

Giữ baud, độ dài bản tin, delay, priority, yield và chiều dài queue như cũ.
Heap lượt 4 writer là 8 KiB; lượt 8 writer tăng lên 12 KiB để đủ cấp phát.
Mỗi lượt vẫn lấy 200 mẫu thành công.

| Cách | TB: 4 → 8 writer (µs) | Max: 4 → 8 writer (µs) | Mức tăng TB (µs) |
|---|---:|---:|---:|
| Mutex | 13408 → 29234 | 15916 → 32247 | +15826 |
| Critical | 3485 → 3484 | 3487 → 3487 | −1 |
| Gatekeeper | 3511 → 3511 | 3514 → 3513 | 0 |

**Mutex bị ảnh hưởng nhiều nhất theo latency đã định nghĩa**, do thời gian chờ
mutex tăng. Critical/gatekeeper gần như không đổi: writer chủ yếu chờ CPU trước
khi vào hàm gửi, phần chờ này nằm ngoài phép đo. 

Log 8 writer có đủ T0–T7 và nội dung A–H, không trộn hoặc gián đoạn `seq` giữa
các bản tin đầy đủ. RAM thêm vẫn là 80 / 0 / 1384 byte vì số đối tượng guard
không đổi; stack/TCB của các writer bổ sung nằm ngoài phép đo RAM này.

Ảnh đo 8 writer: [mutex](measurements/mutex/8writers_watch.png),
[critical](measurements/critical/8writers_watch.png),
[gatekeeper](measurements/gatekeeper/8writers_watch.png);
log tương ứng: `8writers_uart_log.log` trong mỗi thư mục.
