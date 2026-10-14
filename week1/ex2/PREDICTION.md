# Dự đoán — Week 1 / Ex2

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412108    Ưu tiên của tôi: A=4 B=2 C=3

1. Bước 2 (`configUSE_PREEMPTION`=1, ưu tiên khác nhau): task nào chạy? task nào
   **không bao giờ** chạy? CPU share dự đoán:

   | Task | CPU share dự đoán (%) |
   |---|---|
   | A | 100 |
   | B | 0 |
   | C | 0 |

2. Bước 3 (`configUSE_PREEMPTION`=0): dự đoán thay đổi gì so với bước 2? Vì sao?  
Trả lời: không khác gì cả bởi vì dù bật hay tắt preemption thì Task A, với ưu tiên cao nhất không bao giờ block/yield nên các Task khác không bao giờ được chạy (starvation).
3. Bước 4 (3 task cùng ưu tiên): time slice dự đoán = ______ ms. Dựa vào đâu?  
Trả lời: Time Slice dự đoán = 1ms. Dựa vào thời gian 1 Tick của kernel (1/configTICK_RATE_HZ)
4. Bước 5: tôi sẽ sửa starvation bằng cách: ______ . Cách này đánh đổi gì?  
Trả lời: Sửa bằng cách bật Preemption + viết thêm vTaskDelay(1) vào mỗi Task. Đánh đổi là các Task không chạy được liên tục, sẽ có khoảng 340us CPU ở trong IdleTask.
5. Chân nào sẽ không bao giờ đổi trạng thái trên logic analyzer ở bước 2? ______  
Trả lời: TASK_B_PIN và TASK_C_PIN, tức (PA5 và PA7).
