# Dự đoán — Week 3 / Ex2

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412108

1. Với nút nhấn cơ khí thông thường, nảy phím thường kéo dài bao lâu
   (tra cứu/ước lượng)? So với `DEBOUNCE_MS` của bạn: đủ lớn để lọc hết không?  
   - Trả lời: Với nút nhấn cơ khí thông thường, nảy phím thường kéo dài 1-20ms. Tuy nhiên với cách debounce trong bài này, con số này dự đoán sẽ không ảnh hưởng tới hiệu quả debounce bởi vì `DEBOUNCE_MS` luôn được tính từ cạnh nhiễu cuối cùng xuất hiện. Tuy nhiên, `DEBOUNCE_MS` phải đủ dài để tránh coi khoảng yên tạm thời giữa các cạnh nảy là trạng thái ổn định.
2. Nếu `DEBOUNCE_MS` quá lớn, điều gì xảy ra khi người dùng nhấn nhả rất
   nhanh liên tiếp (double-click)?  
   - Trả lời: nếu `DEBOUNCE_MS` quá lớn và người dùng double click đủ nhanh (trước khi hết thời gian debounce) thì Software Timer sẽ bị reset trước khi CallBack được gọi. Nếu doubleclick liên tục thì khả năng là tầng ứng dụng của firmware sẽ không bao giờ đọc được event của nút.
3. Khi callback của `debounce_timer` bị block 50 ms (thử nghiệm bước 4), bạn
   dự đoán `longpress_timer` sẽ bị trễ bao nhiêu? Giải thích bằng cơ chế
   daemon task + queue lệnh, không chỉ đoán số.  
   - Trả lời: việc Callback của `longpress_timer` có bị trễ không và trễ bao nhiêu còn phụ thuộc vào số cấu hình cụ thể. Ví dụ trường hợp Callback của `debounce_timer` chạy ở tick 10, và bị block trong 50ms, nếu Callback của `longpress_timer` cần chạy ở tick 30 thì `longpress_timer` sẽ bị trễ (10 + 50 - 30) = 30ms. Nhưng ở trong cấu hình hiện tại, `debounce_timer`(15ms) và `longpress_timer`(600ms) được start cùng lúc khi nút được nhấn, nếu `debounce_timer`bị block trong 50ms thì callback của nó sẽ chạy xong tại khoảng 65ms, lúc này `longpress_timer` vẫn chưa cần chạy, đồng thời tick hệ thống vẫn được cập nhật đủ nên `longpress_timer` sẽ không bị trễ.
4. `LONGPRESS_MS` của bạn là bao nhiêu? Giữ phím đúng ngưỡng này, bạn dự đoán
   callback `longpress_callback` chạy lần đầu ở thời điểm nào tính từ lúc
   nhấn xuống?  
   - Trả lời: `LONGPRESS_MS` hiện tại là 600ms. Nếu không có nảy và nút vẫn được giữ, Callback `longpress_callback` đầu tiên dự kiến chạy khoảng 600 ms sau cạnh xuống.
