# Dự đoán — Week 2 / Ex2

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412108    `WRITER_COUNT` = 4   `MSG_LEN` = 40

1. Ở `UART_GUARD_MODE = 0`: dự đoán lỗi lẫn lộn sẽ xuất hiện sau khoảng bao
   nhiêu dòng log? Vì sao MSG_LEN và WRITER_PERIOD_MS của bạn ảnh hưởng đến
   con số này?  
   - Theo cách viết firmware hiện tại thì em dự đoán lỗi lẫn lộn sẽ xuất hiện ngay dòng log đầu tiên. Và cũng theo firmware hiện tại, MSG_LEN và WRITER_PERIOD_MS không ảnh hưởng đến việc lỗi xuất hiện ở log đầu tiên, có chăng là vì MSG_LEN > 1 nên mới có hiện tượng bản tin bị cắt ngang, trộn thôi.
2. Dự đoán thứ tự **độ trễ in** (latency) từ thấp đến cao giữa mutex / critical
   section / gatekeeper. Vì sao?  
   - Critical < Mutex < Gatekeeper
   - Critical: dự đoán latency thấp và ít dao động nhất. Gửi liền cả bản tin, không bị task khác chen vào; đổi lại, các task và ngắt bị che phải chờ
   - Mutex: có thể cao hơn critical vì Task phải chờ lấy mutex và trong lúc gửi vẫn có thể bị ngắt hoặc chuyển task. Việc yield sau mỗi byte cũng tạo thêm độ trễ dù các task khác vẫn không thể chen vào UART.
   - Gatekeeper: có thêm thao tác queue và chuyển task; bản tin còn có thể phải chờ các bản tin trước gửi xong. Tuy nhiên, keeper có priority cao hơn writer và gửi liền cả mảng, nên cũng chưa chắc chắn sẽ chậm hơn mutex.
3. Với critical section, task nào trong hệ thống có nguy cơ bị ảnh hưởng xấu
   nhất nếu đoạn in quá dài? (gợi ý: nghĩ đến ngắt, không chỉ task)  
   - Nếu critical section quá dài thì sẽ ảnh hưởng đến tất cả các task có yêu cầu thời gian thực nghiêm ngặt. Lí do là vì trong critical section, cả Tick của Kernel lẫn HAL Tick đều không được cập nhật do SysTick Handler và ngắt của TIM4 có ưu tiên thấp nên không chen vào được.
4. RAM dùng thêm của gatekeeper so với mutex: dự đoán tăng hay giảm, vì sao?  
   - Gatekeeper cần nhiều RAM hơn vì cần một Stack + TCB cho Task riêng, ngoài ra còn cần thêm queue để chứa 4 bản tin.
5. Nếu `WRITER_COUNT` tăng gấp đôi, cách nào trong 3 cách bị ảnh hưởng về độ trễ
   nhiều nhất? Vì sao?  
   - Em dự đoán là Mutex có khả năng bị ảnh hưởng nhiều nhất do thêm tranh chấp khóa và overhead chuyển task.
