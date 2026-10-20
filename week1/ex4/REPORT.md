# Report — Week 1 / Ex4

`student_id`: 2412108    Commit của PREDICTION.md: ddaeeb3

## 1. Ngân sách RAM từ `.map`
| Mục | Symbol / section trong `.map` | Bytes |
|---|---|---|
| `.data` |.data |96 |
| `.bss` (không kể heap) |.bss − ucHeap |2792 |
| Heap FreeRTOS (`ucHeap`) |.bss.ucHeap |9216 |
| Stack của main (MSP) | _Min_Stack_Size |1024 |
| Heap C tối thiểu | _Min_Heap_Size |512 |
| Còn lại / chưa dùng | — |6840 |
| **Tổng** | | **20480** |

`.map` commit tại: `8e731bc` — [firmware.map](measurements/normal/firmware.map)

## 2. Số lúc chạy (dán nguyên văn log UART)
```
Heap free: 7344 bytes, minimum ever: 7344 bytes
dyn_task: stack free minimum = 135 words (540 bytes)
stat_task: stack free minimum = 135 words (540 bytes)
ram_report: stack free minimum = 70 words (280 bytes)
idle: stack free minimum = 109 words (436 bytes)
timer: stack free minimum = 84 words (336 bytes)
```

| Đại lượng | Words | Bytes |
|---|---|---|
| `xPortGetFreeHeapSize()` | — |7344 |
| `xPortGetMinimumEverFreeHeapSize()` | — |7344 |
| HWM `dyn_task` |135 |540 |
| HWM `stat_task` |135 |540 |
| HWM `ram_report_task` |70 |280 |
| HWM idle task |109 |436 |
| HWM timer task |84 |336 |

Heap còn trống >= `TARGET_FREE_HEAP_BYTES` (4096)? 7344 >= 4096

## 3. Đối chiếu `.map` vs runtime
Heap trong `.map` = 9216 ; `xPortGetFreeHeapSize()` = 7344 ; lệch = 1872 bytes.
Lệch này gồm những gì (liệt kê từng mục kèm số bytes):

| Khoản | Bytes |
|---|---:|
| Stack dyn_task | 640 |
| TCB dyn_task | 76 |
| Stack ram_report_task | 1024 |
| TCB ram_report_task | 76 |
| Metadata/căn chỉnh của bốn block cấp phát | 40 |
| Căn chỉnh đầu vùng heap và block đánh dấu cuối | 16 |
| **Tổng** | **1872** |

## 4. Dynamic vs static
HWM của `dyn_task` và `stat_task` lệch 0 words dù làm cùng một việc. Vì sao?
`xTaskCreate` và `xTaskCreateStatic` khác nhau ở đâu về **nơi** TCB và stack nằm?  
Trả lời:  
- Trong bản normal, cả hai task cùng chạy shared_work(), được cấp 160 words và đo được HWM 135 words, nên chênh 0 words. Cách cấp phát dynamic/static không tự làm công việc của task cần nhiều stack hơn.  
- dyn_task: kernel lấy TCB và stack bên trong ucHeap bằng cấp phát động.  
- stat_task: dùng statTaskTCB và statTaskStack bạn khai báo sẵn, nằm trong .bss, ngoài ucHeap.

## 5. Tràn stack
Log/ảnh chứng minh hook đã chạy:
```
FAULT: stack overflow
```

[Log UART tràn stack](measurements/overflow/uart_log.log)  

Tại sao trong `vApplicationStackOverflowHook` không được gọi hàm UART blocking
(hoặc: nếu bạn vẫn gọi được, giải thích tại sao nó không crash — và tại sao đó là may): Vì trong hàm HAl có cơ chế timeout theo Hal Tick mà Hook đã tắt ngắt nên nếu trường hợp dữ liệu không gửi được thì sẽ bị treo.

## 6. Stack nhỏ nhất an toàn
| Lần thử | Stack (words) | HWM (words) | Kết quả |
|---|---|---|---|
| 1 — có hog | 160 | — | FAULT: stack overflow |
| 2 — có hog | 212 | 25 | 25 < 32 |
| 3 — có hog | 220 | 33 | 33 >= 32 |  

[Log UART stack220](measurements/stack220/uart_log.log)  

Giá trị cuối cùng chọn: 220 words (880 bytes). Biên an toàn: 33 words (132 bytes). Lý do chọn biên đó: khi chạy dyn_task với mảng hog 640 bytes, HWM đo được còn 33 words, đạt biên tối thiểu 32 words theo yêu cầu bài.



## 7. Lệch so với dự đoán
Mỗi dòng lệch + nguyên nhân.

| Đại lượng | Dự đoán | Thực tế | Thực tế − dự đoán |
|---|---:|---:|---:|
| Heap trống normal (bytes) | 7400 | 7344 | -56 |
| HWM dyn normal (words) | 140 | 135 | -5 |
| HWM stat normal (words) | 140 | 135 | -5 |
| HWM Idle normal (words) | 108 | 109 | +1 |
| HWM timer normal (words) | 100 | 84 | -16 |
| Stack dyn có hog, mức cuối đã kiểm chứng (words) | 212 | 220 | +8 |  

Lí do lệch:  
- Heap: Dự đoán chỉ trừ TCB/stack mà chưa xét tới phần bytes cho metadata, căn chỉnh và phần quản lý heap_4.  
- HWM dyn/stat/Idle/timer và Stack dyn có hog: các giá trị dự đoán chỉ là ước lượng mức stack mà code sẽ dùng chứ chưa có cơ sở tính toán chính xác về độ sâu gọi hàm, biến cục bộ, thanh ghi/context và cách compiler sinh mã nên dẫn đến sai số.  


## 8. Câu hỏi đánh đổi
Với `HEAP_SIZE_BYTES` = 9216 của bạn: nếu phải thêm một task nữa có stack 128 words,
**lấy RAM từ đâu**? Dựa vào bảng ngân sách của bạn, đưa ra 2 phương án kèm chi phí
cụ thể (bytes), rồi chọn một và **thực hiện, đo lại để chứng minh**.

| Phương án | Stack (bytes) | TCB (bytes) | Metadata/căn chỉnh cấp phát (bytes) | Tổng (bytes) |
|---|---:|---:|---:|---:|
| Dynamic — đã đo | 512 | 76 | 12 | 600 |
| Static — tính từ kích thước TCB và stack | 512 | 76 | — | 588, chưa kể padding bố trí |

| Số đo phương án dynamic | Giá trị |
|---|---:|
| Heap trống trước khi thêm task, dyn có hog và stack220 | 7104 bytes |
| Heap trống sau khi thêm task | 6504 bytes |
| Heap trống thấp nhất sau khi thêm task | 6504 bytes |
| Heap giảm | 600 bytes |
| HWM add_task | 103 words = 412 bytes |
| HWM dyn_task | 33 words = 132 bytes |
| TARGET_FREE_HEAP_BYTES | 4096 bytes |

[Log UART thêm task](measurements/add_task/uart_log.log).  
Em chọn cấp phát dynamic vì heap FreeRTOS còn đủ, không cần tăng HEAP_SIZE_BYTES hay dành thêm TCB/stack static. Sau khi thêm task có stack 128 words, heap trống giảm từ 7104 xuống 6504 bytes, tức 600 bytes, vẫn lớn hơn yêu cầu 4096 bytes. HWM task mới còn 103 words; dyn_task vẫn còn 33 words. Log UART kèm theo chứng minh phương án đã được thực hiện và đo lại.
