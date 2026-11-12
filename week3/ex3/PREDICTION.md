# Dự đoán — Week 3 / Ex3

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412108

1. Thời gian truyền 1 byte ở `RX_BAUD` của bạn là bao nhiêu us? (8N1, tính cả
   start/stop bit). Trong thời gian đó `parser_task` có kịp xử lý 1 byte từ
   stream buffer không, nếu nó là task ưu tiên cao nhất hệ thống?  
   - Trả lời: Với UART 115200 baud, cấu hình 8N1, một byte cần 10 bit nên thời gian truyền khoảng 86,8 µs. Ở CPU 72 MHz, dự đoán là parser xử lý kịp 1 byte bởi vì parser trong bài này hầu như chỉ có đọc byte ra từ stream buffer và ghép và thực thi lệnh ngắn.
2. `STREAM_BUFFER_BYTES` của bạn chứa tối đa bao nhiêu lệnh dài `MAX_CMD_LEN`
   byte trước khi đầy?  
   - Trả lời: Stream buffer 128 byte chứa được tối đa 4 lệnh, mỗi lệnh gồm 31 byte nội dung và một byte kết thúc \r.
3. Nếu `xStreamBufferSendFromISR` trả về 0 (stream buffer đầy) trong
   `uart_rx_isr_handler`, byte đó sẽ bị mất. Theo bạn, trường hợp nào thực tế
   khiến điều này xảy ra (gửi quá nhanh / `parser_task` ưu tiên quá thấp /
   cái khác)?  
   - Trả lời: nếu byte bị mất thì là do tốc độ nhận byte lớn hơn tốc độ parser lấy dữ liệu trong một khoảng đủ dài, việc này có thể là kết quả của việc parser bị task cao hơn chiếm CPU hoặc xử lý lệnh quá lâu.
4. Dự đoán: gửi lệnh bằng gõ tay bình thường có mất byte không? Gửi bằng
   script gửi liên tục không nghỉ thì sao?  
   - Trả lời: gõ tay thì thường là khá chậm so với tốc độ xử lý của CPU nên khả năng cao là sẽ không mất nếu firmware được thiết kế hợp lý, không có lỗi gây nghẽn nghiêm trọng. Còn nếu dùng script thì cũng tùy thuộc vào tốc độ gửi của script, trong bài này thì nhiều khả năng là khi nhận thì vẫn nhận đủ tuy nhiên queue cho quá trình gửi đi có thể bị đầy vì phản hồi dài hơn lệnh nhận.
