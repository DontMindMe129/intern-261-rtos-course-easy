# Report — Week 2 / Ex4

`student_id`: 2412108    Commit của PREDICTION.md: a45ec53

## 1. Bảng so sánh RAM

| | Bytes thực tế | Cách tính |
|---|---|---|
| Event group (3 điểm đồng bộ) | 40 byte heap; cấu trúc 28 byte | Heap trống trước − sau `xEventGroupCreate()`; `sizeof(StaticEventGroup_t)` |
| Event group (2 điểm) + notification (1 điểm) | 40 byte heap; notification thêm 0 byte | Giữ nguyên object event group; heap trước − sau lần `xTaskNotifyGive()` đầu tiên bằng 0 |
| Chênh lệch | 0 byte heap | 40 − 40 = 0 |

Notification có giá trị/trạng thái sẵn trong TCB nên không cấp phát object mới. Giảm số bit không giảm kích thước event group. Bảng không tính stack/TCB, biến handle và bộ đo.

Ảnh: [event3](measurements/event3/Screenshot%202026-10-24%20215617.png), [notify3](measurements/notify3/watch_notify_10ms_200samples.png).

## 2. Bảng so sánh tốc độ đánh thức

| Cách | Thời gian đánh thức (cycle) | Thời gian đánh thức (us) |
|---|---|---|
| Event-group-bit (producer_1 hoặc 2) | TB = max = 1158 | TB = max ≈ 16,083 |
| Task notification (producer_0) | TB = max = 975 | TB = max ≈ 13,542 |

Mô tả cách đo (2 điểm DWT: lúc gọi set/notify, lúc sync_task bắt đầu chạy):

**Trả lời:** DWT lấy `t0` ngay trước API báo hiệu và `t1` khi hàm chờ trả về trong sync. Độ trễ = `t1 − t0`; CPU 72 MHz nên µs = cycles / 72. Mỗi loại lấy 200 mẫu thực sự đánh thức sync; bỏ lượt tín hiệu đã có sẵn và timeout. Số đo gồm overhead instrumentation quanh API.

Event lấy từ config 1 (producer ngẫu nhiên 2–4 ms); notification lấy từ config 2, riêng ID 0 delay 10 ms để sync kịp quay lại block. Notification nhanh hơn khoảng 2,542 µs trong hai lượt đo này.

Ảnh kết quả ở mục 1; [log config 2](measurements/notify3/uart_log.log) có `bits=0x000006`, `notify=1`, không timeout.

## 3. Khi nào KHÔNG dùng được notification

Tình huống bạn dự đoán ở PREDICTION.md, kiểm chứng lại: đúng hay sai, vì sao.

**Trả lời:** Đúng, đã kiểm chứng bằng config 5 với hai task A/B cùng chờ:

- Event group: set một bit đúng một lần, cả A/B được đánh thức.
- Notification: A/B chờ notification riêng; chỉ notify A thì A được đánh thức, B timeout sau 80 ms.

```text
MULTI event_A=1 event_B=1 notify_A=1 notify_B=0 timeout_B=1 pass=1
```

Vì vậy không thể thay trực tiếp một lần phát sự kiện chung cho nhiều task bằng một notification; muốn báo cả A/B phải gửi riêng.

Bằng chứng: [log](measurements/multi_waiters/uart_log.log), [Watch](measurements/multi_waiters/watch_multi_waiters.png).

## 4. Timeout

Log/ảnh một lần `sync_task` bị timeout (cố tình làm 1 producer chậm hơn
`SYNC_TIMEOUT_MS`), và giá trị trả về của `xEventGroupWaitBits` lúc đó.

**Trả lời:** Config 3 cho ID 2 delay 160 ms, trong khi sync chờ tối đa 80 ms:

```text
TIMEOUT t=80ms bits=0x000003 notify=0
SYNC t=160ms period=0ms bits=0x000007 notify=0
```

Timeout trả về `0x03`: có bit 0/1 nhưng thiếu bit 2. Phân biệt bằng `(bits & 0x07) == 0x07`: đúng là đủ bit, sai là timeout khi chưa đủ điều kiện. Các bit đã có được giữ sau timeout; khi bit 2 đến, lần chờ sau thành công và xóa bit.

Đã ghi 40 TIMEOUT / 40 SYNC: [log](measurements/timeout3/uart_log.log), [Watch](measurements/timeout3/watch_timeout3.png).

## 5. Lệch so với dự đoán

Mỗi mục lệch + nguyên nhân vật lý.

**Trả lời:** RAM 28 byte cấu trúc / 40 byte heap, notification thêm 0 byte và so sánh latency đều khớp dự đoán.

## 6. Câu hỏi đánh đổi

Với `EVENT_BIT_COUNT` của **bạn**, nếu tăng thêm 1 bit (1 producer nữa) thì chu
kỳ đồng bộ trung bình đo được thay đổi thế nào? Trả lời bằng số từ dữ liệu của
bạn, rồi đo lại để kiểm chứng.

**Dự đoán (bổ sung sau lượt đo):** Chu kỳ đồng bộ tăng nhẹ hoặc gần như không đổi, vì các producer delay đồng thời; nguồn thứ tư không nhất thiết đến cuối.

**Kết quả đo:** So 500 giá trị `period` đầu tiên sau dòng `period=0` trong mỗi log:

| Cấu hình | Chu kỳ TB | Min–max |
|---|---:|---:|
| 3 producer | 6,968 ms | 6–8 ms |
| 4 producer | 7,000 ms | 6–8 ms |
| Chênh lệch | +0,032 ms, khoảng 0,46% | |

Tăng từ 3 lên 4 producer làm chu kỳ trung bình tăng 0,032 ms, gần như là không có thay đổi nhiều.


Dữ liệu: [log 3 producer](measurements/event3/uart_log.log), [log 4 producer](measurements/event4/uart_log.log), [Watch 4 producer](measurements/event4/watch_event4.png).
