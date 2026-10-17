# Dự đoán — Week 1 / Ex3

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412108    `PERIOD_MS` = 23    `WORK_JITTER_US` = 600    `SAMPLE_COUNT` = 500

1. Bảng dự đoán (điền trước khi đo):

| # | Delay | Tick | Chu kỳ tb (ms) | Jitter (us) | Drift tổng sau SAMPLE_COUNT (ms) |
|---|---|---|---|---|---|
| 1 | `vTaskDelay` | 1000 | 23 | có thể gần bằng 0 | 0 |
| 2 | `xTaskDelayUntil` | 1000 | 23 | có thể gần bằng 0 | 0 |
| 3 | `vTaskDelay` | 100 | 20 | có thể gần bằng 0 | -1500 |
| 4 | `xTaskDelayUntil` | 100 | 20 | có thể gần bằng 0 | -1500 |

2. Cấu hình nào có drift lớn nhất? Vì sao, viết bằng công thức có `WORK_JITTER_US`.  
   Trả lời: Cấu hình 3 và 4 có drift lớn nhất vì với chu kỳ mong muốn là 23ms, mỗi sample sẽ bị sớm hơn 3ms => 500 samples sẽ bị sớm hơn mong muốn 3ms * 500 = 1500ms.
3. `PERIOD_MS` = 23 . Với tick 100 Hz, một tick = 10 ms, nên chu kỳ thực tế sẽ
   bị làm tròn thành 20 ms. Sai số hệ thống: (20 - 23)/23 = -13.04 %, tức hệ thống chạy ngắn hơn khoảng 13%
4. Jitter ở tick 1000 Hz so với 100 Hz: lớn hơn / nhỏ hơn / bằng? Vì sao?  
   Trả lời: Bởi vì bài này chỉ có một task nên em chưa thấy có khả năng nào làm cho jitter khác nhau ở 2 tần số trên
5. Nếu task này **không** phải ưu tiên cao nhất, đại lượng nào trong bảng bị ảnh hưởng
   nhiều nhất?  
   Trả lời: có thể là Jitter trong trường hợp Task có ưu tiên cao hơn chiếm cpu trong 1 khoảng thời gian dài và không đều.
