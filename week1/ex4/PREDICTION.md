# Dự đoán — Week 1 / Ex4

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412108    `HEAP_SIZE_BYTES` = 9216    `DYN_TASK_STACK_WORDS` = 160
`STATIC_TASK_STACK_WORDS` = 160    `HOG_DEPTH_BYTES` = 640    `TARGET_FREE_HEAP_BYTES` = 4096

1. Ngân sách RAM dự đoán (tổng phải = 20480 bytes):

| Mục | Bytes dự đoán |
|---|---|
| `.data` |96 |
| `.bss` (không kể heap FreeRTOS) |2792 |
| Heap FreeRTOS (`ucHeap`) |9216 |
| Stack của main (MSP) |1024 |
| Heap C tối thiểu | 512 |
| Còn lại / chưa dùng |6840 |
| **Tổng** | **20480** |

2. `xPortGetFreeHeapSize()` ngay sau khi scheduler chạy: 7400 bytes.
   `HEAP_SIZE_BYTES` − số này = 1816 . Số đó là của TCB và stack của hai task dynamic: dyn_task và ram_report.
3. High water mark dự đoán (words): dyn 140 , stat 140 , idle 108 , timer 100 .
4. `dyn_task` có stack 160 words = 640 bytes. Mảng `HOG_DEPTH_BYTES` = 640 bytes.
   Hook tràn stack **có** chạy không? Vì sao?  
   - Trả lời: Stack dyn_task có 160 words = 640 bytes. Mảng HOG_DEPTH_BYTES cũng chiếm 640 bytes, chưa kể context và các lời gọi hàm, nên dự đoán task sẽ tràn stack và hook sẽ được gọi khi kernel phát hiện lỗi.
5. Heap trong `.map` và `xPortGetFreeHeapSize()` sẽ lệch nhau. Lệch vì: TCB + Stack được cấp phát động của 2 Task dyn_task và ram_report. Ngoài ra còn queue timer, metadata và căn chỉnh.
6. Stack nhỏ nhất an toàn của `dyn_task` dự đoán: 212 words. Dựa vào việc phải chừa chỗ cho mảng Hog (160) + khoảng 20 words cho các phần thực thi khác, đồng thời phải đảm bảo biên an toàn 32 words.
