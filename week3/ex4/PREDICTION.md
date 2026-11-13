# Dự đoán — Week 3 / Ex4

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412108

1. Chạy `app/deadlock_demo.c` y nguyên như được cấp. Dự đoán: hệ thống sẽ
   treo sau khoảng bao lâu (ngay lập tức, hay sau vài chu kỳ)? Vì sao không
   phải ngay lập tức (gợi ý: cần cả 2 task "đua" vào đúng thời điểm)?  
   - Trả lời: với code hiện tại thì lúc đầu task_beta và task_alpha sẽ lần lượt take mutex b và mutex a sau đó block trong 5ms. Khi hết 5ms thì task_beta thức dậy, yêu cầu mutex a nhưng chưa có và bị block, task_alpha thức dậy, yêu cầu mutex_b nhưng cũng chưa có và bị block => treo. Vậy dự đoán là hệ thống sẽ treo sau khoảng 5ms
2. Khi treo, `uxTaskGetSystemState` sẽ báo trạng thái gì cho `task_alpha` và
   `task_beta`? Dự đoán tên mutex mỗi task đang chờ (dựa trên đọc code, chưa
   chạy công cụ).  
   - Trả lời: `uxTaskGetSystemState` sẽ báo blocked cho cả task_alpha và task_beta. task_alpha chờ mutex b, task_beta chờ mutex a.
3. Tính reload value cho IWDG với `IWDG_TIMEOUT_MS` của bạn (LSI ~40 kHz).  
   - Trả lời: Chọn prescaler bằng 4. Với LSI  40 kHz và `IWDG_TIMEOUT_MS` 250 ms, giá trị reload:  
   ((250 * 40000)/(1000 * 4)) - 1 = 2499
4. Sau khi sửa lỗi deadlock và thêm supervisor + IWDG, nếu bạn cố tình tái
   tạo lại tình huống deadlock cũ (tạm thời), dự đoán board sẽ phản ứng thế
   nào và sau bao lâu.  
   - Trả lời: với tình huống deadlock cũ và IWDG chạy ngay từ đầu thì board sẽ bị reset sau khoảng 250ms.
