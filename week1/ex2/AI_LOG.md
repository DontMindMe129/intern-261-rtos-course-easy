# AI_LOG.md — Week 1 / Ex2

`student_id`: 2412108  
AI dùng: ChatGPT/Codex  
Ngày tổng hợp: 2026-10-14

Log được tổng hợp từ các lượt trao đổi trong phiên học này. Cột prompt tóm tắt câu hỏi, không phải bản chép nguyên văn. Các lượt hỏi nối tiếp cùng nội dung được gom thành một mục và liệt kê trong phần tóm tắt. Kiến thức tích hợp STM32/FreeRTOS dùng lại từ ex1 được ghi chi tiết trong AI_LOG của ex1.

Người học đã báo build ex2 thành công; chưa flash hoặc đo ex2 tại thời điểm tổng hợp. CPU share, time slice và phương án B5 là dự đoán/thảo luận, không phải số đo thực nghiệm. Code hiện tại là cấu hình B2. AI chỉ đọc và hướng dẫn trong quá trình học; người học tự sửa code. Hai file AI_LOG được AI tạo theo yêu cầu riêng của người học, không kèm commit hoặc flash.

| # | Prompt (tóm tắt) | AI dùng | Mình giữ lại gì / sửa gì |
|---|---|---|---|
| 1 | Tạm dừng ex1, chuyển ex2; dùng lại phần nào của ex1, có nên tạo template chung không? | ChatGPT/Codex | Dùng lại nền CubeMX/FreeRTOS, không dùng logic blink. Tạo firmware riêng cho ex2; chưa tạo template riêng vì lợi ích hiện tại hạn chế. |
| 2 | Drivers báo hàng loạt lỗi là gì? | ChatGPT/Codex | AI phát hiện thiếu dấu phẩy trong workspace settings; người học báo đã hết lỗi sau sửa. Không sửa Drivers để chữa lỗi editor. |
| 3 | Busy-loop là gì; concept ex2 gồm những gì? | ChatGPT/Codex | Hiểu vòng chờ giữ task muốn dùng CPU, không block; học priority, starvation, preemption và time slicing. |
| 4 | Tại sao bật preemption vẫn starvation; tắt preemption không chữa được thì thí nghiệm có vô nghĩa không? | ChatGPT/Codex | Với ba task tạo trước scheduler và không block/yield, A có thể chạy mãi ở cả hai chế độ. B3 kiểm tra ngộ nhận nhưng không minh họa mạnh khác biệt preemption. |
| 5 | Tạo busy-loop bằng HAL_Delay không; cả ba task giữ busy-loop xuyên suốt bài không? | ChatGPT/Codex | Dùng DWT theo đề, không HAL_Delay. Giữ phần công việc busy-wait qua B2–B5; B5 thêm block sau công việc. |
| 6 | Bắt đầu từ đâu; kiểm tra bộ khung ex2 sau khi copy/gen. | ChatGPT/Codex | PA1/PA5/PA7 output; clock 8 MHz; configMAX_PRIORITIES = 5; thêm app_main.c, tasks.c, dwt.c và include path app vào CMake. |
| 7 | DWT là gì; có cấu hình trên CubeMX được không, chỉ project này hay nói chung; dwt_init khởi tạo gì? | ChatGPT/Codex | Dùng CYCCNT của core Cortex-M3. Khởi tạo bằng code: bật trace, reset counter, bật CYCCNTENA; không cần thêm ngắt DWT cho vòng chờ này. |
| 8 | Các định nghĩa thanh ghi DWT/CoreDebug nằm ở file nào? | ChatGPT/Codex | Tra core_cm3.h qua chuỗi stm32f1xx.h -> stm32f103xb.h -> core_cm3.h; phân biệt thanh ghi phần cứng với biến phần mềm. |
| 9 | dwt.c không nhận macro dù đã include; mỗi lần chuyển bài phải đổi cấu hình editor không; có cách tiện hơn không và chọn project ở đâu? | ChatGPT/Codex | Cấu hình editor ban đầu trỏ ex1; cần database của ex2. Thảo luận chọn nhiều project với CMake Tools làm configuration provider để tránh đổi đường dẫn mỗi lần. |
| 10 | TODO2 của dwt.c yêu cầu làm gì? | ChatGPT/Codex | Chuyển microseconds thành CPU cycles, lấy CYCCNT đầu và chờ bằng phép trừ unsigned để xử lý wrap-around. |
| 11 | Kiểm tra công thức microseconds/(1/configCPU_CLOCK_HZ); công thức nhân trước/chia sau được không; kiểm tra lại code. | ChatGPT/Codex | Sửa lỗi chia số nguyên về 0. Code hiện dùng microseconds * (configCPU_CLOCK_HZ / 1000000U), phù hợp 8 MHz và các khoảng 120–300 us. |
| 12 | Ba task có phải HIGH, busy, LOW, busy rồi HIGH không; hạ LOW để làm gì? | ChatGPT/Codex | Theo đề chỉ busy-wait khi HIGH, LOW rồi lặp ngay. LOW đánh dấu kết thúc lượt công việc; mức GPIO không phải dấu Running chính xác khi có preemption. |
| 13 | Kiểm tra tasks.c trước và sau khi viết helper. | ChatGPT/Codex | Sửa số chân thành mask 1U << TASK_x_PIN. Viết helper chung để không copy-paste ba thân xử lý. |
| 14 | task.h của ex1 nằm ở đâu; tham số cuối xTaskCreate là gì? | ChatGPT/Codex | task.h nằm trong include của kernel. Tham số cuối nhận địa chỉ biến handle hoặc NULL; handle khác trạng thái trả về. |
| 15 | Kiểm tra phần tạo task nhiều lần sau khi sửa. | ChatGPT/Codex | Sửa lỗi tạo task_a ba lần; sửa priority B/C từ 4 thành 2/3, sau đó dùng TASK_A_PRIO/TASK_B_PRIO/TASK_C_PRIO. |
| 16 | Kiểm tra main, header, hook; bước tiếp theo và config đã đúng chưa? | ChatGPT/Codex | Nối app_main từ main, thêm hai hook lỗi bộ nhớ. Config B2 dùng preemption/time slicing bật, tick 1000 Hz và đủ năm mức priority. Stack 128 words/task chưa đo xác nhận. |
| 17 | Build được rồi, làm prediction; CPU share là gì? | ChatGPT/Codex | CPU share là tỷ lệ thời gian CPU chạy task. Dự đoán A gần toàn bộ thời gian task, B/C 0 ở B2; không đồng nhất GPIO HIGH với thời gian chiếm CPU. |
| 18 | Các trường hợp chỉ sửa config rồi chạy board kiểm tra đúng không? | ChatGPT/Codex | B3 đổi config; B4 đổi priority khi tạo task; B5 thêm delay vào task. Mỗi cấu hình phải build/flash/đo riêng sau khi commit prediction. |
| 19 | Phân tích B4; time slicing có làm ba chân cùng HIGH không; có thể bị preempt giữa LOW và HIGH không? | ChatGPT/Codex | Time slice danh nghĩa 1 ms khi cùng priority và luôn Ready. GPIO có thể giữ HIGH hoặc LOW khi task không chạy; DWT tiếp tục tăng trong lúc task khác chạy. |
| 20 | Phân tích B5; delay nhỏ hơn 1 được không; vTaskDelay dùng ms hay tick? | ChatGPT/Codex | Đề xuất vTaskDelay(1) sau mỗi lượt cả ba task, giữ priority ban đầu. Tham số là tick nguyên; delay 0/yield không chữa starvation của task thấp. Chưa thêm vào code B2. |
| 21 | Kiểm tra prediction và code sau khi thảo luận delay. | ChatGPT/Codex | AI góp ý A=100% là lý tưởng bỏ qua ISR; time slice cần cả preemption/time slicing; 340 us là phần còn lại cho Idle và overhead dưới giả định, không phải thời gian Idle đã xác nhận. |
| 22 | Chưa flash được thì ex2 còn làm gì? | ChatGPT/Codex | Có thể làm prediction, report lý thuyết và kiểm tra API trước; phần đo, CPU share, time slice và kiểm chứng B5 vẫn chưa hoàn tất. |
| 23 | Dò các điều kiện bắt buộc chưa khớp. | ChatGPT/Codex | Code B2 khớp đề chi tiết; còn commit prediction, AI_LOG, kết quả check_api và phần thực nghiệm. Phát hiện SYLLABUS liệt kê taskYIELD/vTaskPrioritySet nhưng README/rubric ex2 chưa nói rõ cách dùng. |
| 24 | AI_LOG có riêng từng ex không, có phải ghi mọi thứ; yêu cầu ghi lại log ex1/ex2. | ChatGPT/Codex | Tạo log riêng theo bài; tổng hợp các câu hỏi, kiểm tra code và phần đã áp dụng, không dùng câu trả lời AI thay bằng chứng đo. |

## Những giới hạn và việc cần bổ sung

- Chưa có số đo thực tế, bốn ảnh logic analyzer hoặc hai thời điểm preemption được xác nhận ở B4.
- Không suy CPU share trực tiếp từ thời gian GPIO HIGH khi task có thể bị preempt.
- DWT busy-wait đo số chu kỳ đã trôi qua, kể cả lúc ISR hoặc task khác chạy; không đo riêng CPU time của task.
- Comment mẫu 72 MHz / wrap-around khoảng 59 s không khớp clock 8 MHz hiện tại; giá trị tương ứng khoảng 537 s.
- Prediction vẫn cần làm rõ các giả định A gần 100% và phần còn lại 340 us, đồng thời commit trước lần flash đầu.
- Cần chạy check_api.sh và lưu kết quả; kiểm tra source bằng AI không thay thế kết quả script.
- SYLLABUS yêu cầu thêm taskYIELD/vTaskPrioritySet nhưng đề chi tiết chưa thống nhất. Chưa tự thêm hai API vào B2 để tránh thay đổi thí nghiệm.
- Trong câu đánh đổi của report, còn phải tăng TASK_C_WORK_US gấp ba và đo kiểm chứng sau này.
