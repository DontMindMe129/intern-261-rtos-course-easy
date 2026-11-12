# Ex3 — Cách kiểm thử sau khi nạp

Chưa có kết quả phần cứng. Điền/commit PREDICTION.md trước khi flash.

## Kết nối và giao thức

- USB–UART TX -> PA10, RX <- PA9, GND chung; logic UART 3.3 V.
- USART1: 115200, 8N1. Logic analyzer: PA7; có thể thêm PA9/PA10.
- `P100\r`, `S2\r`, `T0\r`, `T1\r` vẫn hoạt động như lệnh gõ tay.
- Lệnh đo có thêm số thứ tự: `P10#42\r`. Số thứ tự là uint32 thập phân.
- Sau khi kiểm tra/áp dụng, firmware trả `OK P10#42\r\n`.
  Echo đầy đủ giúp kiểm tra cả nội dung lệnh, không chỉ số thứ tự.
- Lệnh sai trả `ERR command`; lệnh quá dài trả `ERR length` và bị bỏ đến CR.
- Tối đa 31 byte nội dung (kể cả #sequence), không tính CR. Chấp nhận CRLF.
- `?\r` là lệnh phụ chỉ đọc bộ đếm, trả hai dòng `ST ...`.
- `READY` báo parser đã bật RX; không gửi lệnh trước khi firmware sẵn sàng.

ACK xác nhận đã cập nhật yêu cầu, không chứng minh thời điểm GPIO đã thay đổi.
`P` không hủy lần delay đang diễn ra. PA7 toggle mỗi khoảng delay: chu kỳ một
sóng vuông đầy đủ xấp xỉ hai lần giá trị P. `T1` không reset task từ đầu.

## Chạy script trên máy tính

Đóng terminal khác đang giữ COM. Thay COM5 bằng cổng thực tế:

```powershell
python -m pip install pyserial
python week3/ex3/tools/test_uart_rx.py --port COM5 --count 100 --interval-ms 20 --output week3/ex3/measurements/normal.json
python week3/ex3/tools/test_uart_rx.py --port COM5 --count 1000 --interval-ms 0 --output week3/ex3/measurements/burst.json
```

Script đọc phản hồi đồng thời khi gửi, không đợi ACK từng lệnh nên vẫn tạo được
tải burst. Kết quả lưu lệnh gửi/nhận đúng, số thứ tự thiếu, phản hồi trùng/sai,
phản hồi ERR và delta các bộ đếm. Exit 0 chỉ khi các kiểm tra đều sạch; exit 1
khi có mất/lỗi; exit 2 khi không kết nối/không lấy được thống kê.

`rx` đếm byte tới callback HAL; `drop` chỉ đếm byte không đưa được vào stream
buffer; `uart` đếm lần callback lỗi UART (không phải số byte mất); `rearm` là
lỗi khởi động lại RX sau lỗi. `ok/bad/long` đếm lệnh hợp lệ/sai/quá dài.
`txdrop/txerr` phân biệt mất phản hồi với mất dữ liệu RX. Delta rx có thêm 2
byte của truy vấn cuối `?\r`; script đã tính phần này trong điều kiện clean.
Chỉ dùng script này trên cổng trong lúc thử, không gõ thêm từ nguồn khác.

UART TX dùng queue riêng, không block parser. Tuy nhiên ACK dài hơn lệnh RX:
gửi liên tục có thể làm đầy queue TX ngay cả khi RX không mất byte. Khi đó
`txdrop > 0`, số lệnh được xác nhận có thể ít hơn `ok`; không kết luận là RX
mất byte. Hãy dùng số liệu thực tế và giảm tốc gửi khi cần xác định ngưỡng.
Nếu một byte RX mất, frame có thể sai; echo/counters giúp phát hiện, nhưng
firmware không bảo đảm phục hồi nguyên lệnh đã hỏng.

## Các lượt cần làm

1. Gõ tay P/S/T: ghi log và capture PA7 trước/sau. Với S có thể xem priority
   trong debugger; GPIO một mình không chứng minh priority đã thay đổi.
2. Gửi 100 lệnh có khoảng nghỉ, sau đó burst 1000 lệnh; giữ nguyên baud/tải.
3. Thử lệnh sai (`P0`, `S5`, `T2`, số tràn), lệnh quá dài rồi lệnh hợp lệ:
   kiểm tra ERR và parser đồng bộ lại ở CR.
4. Đo lại cùng tải với `EX3_TEST_DOUBLE_BUFFER` trong experiment_config.h
   (256 bytes). Mặc định `EX3_TEST_NORMAL` là 128 bytes; không sửa params.h.
   Tăng buffer không bảo đảm tăng thông lượng xử lý hoặc tốc độ ACK.
5. UART overrun/recovery: theo dõi các bộ đếm, thử quá tải rồi gửi lại lệnh
   hợp lệ. Không cố tình tạo lỗi điện; không có lỗi thì không ghi đã thử lỗi.

Không có thao tác flash, đo board hay mở COM nào được thực hiện khi tạo source.

## Kiểm tra offline

```powershell
python week3/ex3/tools/test_host.py
```

Kiểm tra trực tiếp parser C bằng host gcc với các hàm HAL/RTOS giả lập:
P/S/T, số thứ tự, số tràn, lệnh sai, CRLF, lệnh quá dài và đồng bộ lại ở CR.
Các test Python kiểm tra mất phản hồi, nội dung bị sai, phản hồi trùng.
Những kiểm tra này không thay thế đo UART, scheduling hoặc recovery trên board.
