# Dự đoán — Week 2 / Ex3

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412108 · `LOW_HOLD_MS = 20` · `MID_WORK_MS = 20`
`HIGH_PERIOD_MS = 50` · `DEADLINE_MS = 25`

1. Cấu hình binary semaphore: thời gian `task_high` bị block dự đoán = 39 ms.
   Phép tính dựa trên đâu (liệt kê các thành phần cộng lại)?  
   - task_high yêu cầu semaphore ở mốc 1ms, low_task còn 19ms busy nhưng bị task_mid preempt mất 20ms nữa nên tổng từ lúc yêu cầu đến lúc task_high lấy được semaphore là khoảng 39ms.
2. `task_high` có đạt `DEADLINE_MS` ở cấu hình binary semaphore không? Vì sao? 
   - Không vì 39ms > 25ms.
3. Cấu hình mutex: thời gian `task_high` bị block dự đoán = 19 ms. Vì sao
   ngắn hơn (hoặc không) so với câu 1?  
   - task_high cũng yêu cầu ở 1ms (task_low còn 19ms busy) nhưng lúc này prio của task_low được tăng lên 3 nên sẽ không mất 20ms bị task_mid preempt => task_high sẽ chỉ bị block 19ms.
4. Trên ảnh logic analyzer ở cấu hình binary semaphore, kênh `task_mid` sẽ ở
   trạng thái nào trong lúc `task_high` bị chặn — chạy liên tục, chạy ngắt
   quãng, hay không chạy? Vì sao?  
   - trong lúc task_high bị chặn thì task_mid sẽ chạy liên tục cho đến khi block vì chỉ có task_high là có prio cao hơn task_mid mà task_high lại đang chờ task_low (có prio thấp hơn task_mid) trả semaphore.
5. Nếu đổi `task_mid` sang ưu tiên **thấp hơn** `task_low`, inversion còn xảy ra
   không? Vì sao (đây là cách phân biệt priority inversion với việc chỉ đơn
   giản là "task_low chạy lâu").  
   - Inversion không còn xảy ra vì task_mid không còn có thể chen vào task_low trong lúc task_high block chờ semaphore nữa.
   - Như vậy, phải phân biệt chờ tài nguyên do Low đang sử dụng với thời gian chờ bị kéo dài do một task trung gian không dùng tài nguyên.

