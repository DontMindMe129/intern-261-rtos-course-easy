# Dự đoán — Week 3 / Ex1

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412108

1. Với `ISR_NVIC_PRIO` của bạn và nhóm priority `NVIC_PRIORITYGROUP_4`, số này
   nằm trong khoảng cho phép (`configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` ..
   `configLIBRARY_LOWEST_INTERRUPT_PRIORITY`) không? Trình bày phép so sánh.  
   - Trả lời: Cấu hình hiện tại: `ISR_NVIC_PRIO` = 8, `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` = 5, `configLIBRARY_LOWEST_INTERRUPT_PRIORITY` = 15 => `ISR_NVIC_PRIO` nằm trong khoảng cho phép gọi API FreeRTOS (5 < 8 < 15).
2. Khi đặt priority EXTI cao hơn ngưỡng cho phép rồi gọi
   `vTaskNotifyGiveFromISR`, bạn dự đoán hành vi quan sát được là gì (reset?
   treo? thông báo gì trên UART/debugger)?  
   - Trả lời: khi đặt prio của EXTI ngoài khoảng cho phép gọi API RTOS thì khi gọi vTaskNotifyGiveFromISR() trong CallBack, Kernel sẽ phát hiện lỗi và gọi configASSERT. trong configASSERT, biến debug được set lên 1, sau đó CPU treo trong vòng lặp vô hạn.
3. Dự đoán latency ISR → task (có `portYIELD_FROM_ISR`) theo đơn vị us, dựa
   trên tần số CPU 72 MHz và số lệnh ước lượng trong đường đi ISR → context
   switch. So với `LATENCY_BUDGET_US`: đạt hay không?  
   - Trả lời: flow là PB1 xuống → CPU vào ISR → HAL xử lý EXTI → gửi notification
          → yêu cầu yield → PendSV chuyển ngữ cảnh
          → ack_task thức dậy → PA7 lên. Ước lượng khoảng 500 lệnh, lấy trung bình 2 cycles/lệnh => khoảng 1000 cycles tương đương 13.9us. So với `LATENCY_BUDGET_US` = 80us thì đạt.
4. Nếu bỏ `portYIELD_FROM_ISR`, `ack_task` sẽ chạy **khi nào** (ngay lập tức
   hay phải chờ tick ngắt kế tiếp)? Latency tăng thêm cỡ bao nhiêu?  
   - Trả lời: Nếu bỏ `portYIELD_FROM_ISR`, `ack_task` sẽ chạy ở SysTick kế tiếp, tùy thuộc vào thời điểm mà notification được gửi đi mà latency có thể tăng thêm từ 0-1ms.
