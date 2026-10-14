# AI_LOG.md — Week 1 / Ex1

`student_id`: 2412108  
AI dùng: ChatGPT/Codex  
Ngày tổng hợp: 2026-10-14

Log được tổng hợp từ các lượt trao đổi trong phiên học này. Cột prompt tóm tắt câu hỏi, không phải bản chép nguyên văn. Các lượt hỏi nối tiếp cùng nội dung được gom thành một mục và liệt kê trong phần tóm tắt. Các trao đổi về tổ chức repo và công cụ có liên quan đến cả ex1/ex2 được ghi ở cả hai log khi cần.

Tại thời điểm tổng hợp, chưa flash hoặc đo ex1. Các giá trị timing và stack được thảo luận là dự đoán; không được dùng thay số đo thực tế. AI chỉ đọc và hướng dẫn trong quá trình học, soạn bản nháp report trong chat; người học tự sửa code. Hai file AI_LOG được AI tạo theo yêu cầu riêng của người học, không kèm commit hoặc flash.

| # | Prompt (tóm tắt) | AI dùng | Mình giữ lại gì / sửa gì |
|---|---|---|---|
| 1 | Với nền tảng baremetal STM32F103, CubeMX, CMake, Ninja, GCC và ST-Link thì bắt đầu RTOS như thế nào? | ChatGPT/Codex | Giữ workflow công cụ; học FreeRTOS API gốc, trạng thái task và scheduling qua thí nghiệm. |
| 2 | Nộp source của repo như thế nào; có gen CubeMX vào app không; bốn tuần dùng chung template ra sao? | ChatGPT/Codex | Phân biệt code CubeMX trong firmware và code ứng dụng trong app; nhận biết repo ban đầu thiếu template và bộ khung build. |
| 3 | Cơ chế chọn bài, thành phần dùng chung/thay đổi, cấu trúc thư mục và lợi ích template là gì? Vì sao gen riêng từng bài vẫn cần template? | ChatGPT/Codex | Chọn project firmware riêng cho từng bài; kernel dùng chung ở third_party. Template chỉ là lựa chọn tái sử dụng nền, không bắt buộc. Chưa tạo template riêng. |
| 4 | Bắt đầu ex1 từ đâu; chạy gen_params.py cần gì; Python có phải compiler không? | ChatGPT/Codex | Cài/kiểm tra Python để chạy script sinh tham số theo MSSV, không dùng Python để compile firmware. Tham số ex1: PA0, 97 ms, stack 128 words. |
| 5 | Có gen project vào week1/ex1/firmware không? | ChatGPT/Codex | Gen project CMake vào firmware, cấu hình PA0 output, HAL timebase TIM4 và SWD. |
| 6 | Source FreeRTOS được compile/link với firmware thành ELF như thế nào; đặt kernel ở đâu; CMake trong firmware có tham chiếu được không? | ChatGPT/Codex | Đặt kernel ở third_party/FreeRTOS-Kernel; hiểu source được compile thành object/thư viện rồi link với code STM32 và ứng dụng. |
| 7 | Giữa add_library(freertos_config INTERFACE) và add_subdirectory cần cung cấp những thông tin gì? | ChatGPT/Codex | Cung cấp include path của FreeRTOSConfig.h, port GCC_ARM_CM3 và lựa chọn heap_4. |
| 8 | Kiểm tra CMake trước/sau khi lưu; include path tương đối tính từ đâu? | ChatGPT/Codex | Sửa include path bị lặp đường dẫn; dùng Core/Inc cho config. Phân biệt file chưa lưu với bản AI đọc trên ổ đĩa. |
| 9 | Vì sao add_subdirectory cần binary_dir khi source ngoài project; có gom source vào firmware không; mỗi CMake phải tạo sản phẩm không; file trung gian có bị xóa không? | ChatGPT/Codex | Hiểu source vẫn ở vị trí cũ, object/thư viện được lưu trong cây build; target quyết định sản phẩm, không phải số file CMake. |
| 10 | CMAKE_CURRENT_BINARY_DIR là gì, vì sao gọi là binary, được chọn ở file nào; dùng /build/Debug hay nguyên biến được không; có phải tạo thư mục trước không? | ChatGPT/Codex | Preset chọn build/Debug; dùng thư mục con freertos-kernel cho kernel. CMake tự tạo thư mục build. |
| 11 | Giải thích FREERTOS_PORT/FREERTOS_HEAP, CACHE STRING; bỏ CACHE được không; mỗi lần đọc CMake nghĩa là gì? | ChatGPT/Codex | Dùng biến thường để cố định port/heap trong CMake. Hiểu configure xử lý CMake, build thực thi mô tả đã sinh. |
| 12 | Qua cấu hình chi tiết; tra macro ở đâu và biết cần khai báo gì bằng cách nào? | ChatGPT/Codex | Tra template_configuration/FreeRTOSConfig.h, FreeRTOS.h và port.c; đối chiếu FreeRTOSConfig.deltas và yêu cầu bài. |
| 13 | configCPU_CLOCK_HZ/configTICK_RATE_HZ nghĩa là gì; tại sao dùng Hz thay vì thời gian một tick; SysTick là timer thì nguồn đếm là gì; khi nào kernel tăng tick? | ChatGPT/Codex | Clock hiện tại là 8 MHz, tick rate 1000 Hz. Port cấu hình SysTick theo clock đầu vào để phát tick danh nghĩa 1 ms. |
| 14 | Giải thích preemption, số mức priority, time slicing và thuật toán chọn task. | ChatGPT/Codex | Ex1 dùng hai mức priority: Idle 0, blink 1; bật preemption/time slicing, tắt port-optimised task selection theo đề. |
| 15 | Giải thích stack Idle, stack task blink, heap, cấp phát động/tĩnh và hook lỗi bộ nhớ. | ChatGPT/Codex | Stack blink lấy từ params.h khi xTaskCreate; stack Idle dùng configMINIMAL_STACK_SIZE. Heap_4 ban đầu 4 KB, chưa đo xác nhận. |
| 16 | Priority ngắt khác priority task như thế nào; dạng thanh ghi là gì; hai macro configLIBRARY_* có tác dụng gì; vì sao thấp nhất là 15 chứ không phải 0? | ChatGPT/Codex | NVIC số nhỏ ưu tiên cao, task số lớn ưu tiên cao. Dịch priority sang các bit có hiệu lực; dùng ngưỡng syscall 5 và mức NVIC thấp nhất 15. |
| 17 | Kiểm tra config, hậu tố U và comment; kiểm tra lại sau khi sửa. | ChatGPT/Codex | Sửa công thức priority, bổ sung tick width 32 bit, tắt Idle/Tick hook, bật INCLUDE_vTaskDelay và bỏ BRINGUP_STACK_WORDS khai báo trùng. |
| 18 | Ánh xạ ba handler nghĩa là gì; handler trong port.c và stm32f1xx_it.c có giống nhau không; vector table ở file nào; phải sửa startup hay đổi tên kernel? | ChatGPT/Codex | Giữ vector table startup; macro đổi tên handler port thành SVC_Handler, PendSV_Handler và SysTick_Handler. Handler CubeMX rỗng khác implementation kernel. |
| 19 | Có cần viết configASSERT ở ex1; yêu cầu ở đâu; macro làm gì; mỗi điều kiện phải define riêng không; điều kiện sai xử lý thế nào? | ChatGPT/Codex | README/SYLLABUS yêu cầu bật assert. Define macro một lần, dùng nhiều điều kiện; chọn dừng để debug khi điều kiện sai. |
| 20 | Vì sao assert nằm trong config, dùng do/while(0); vAssertCalled viết ở đâu; có thể viết thân hàm trong header hay xử lý trực tiếp trong macro không? | ChatGPT/Codex | Chọn xử lý trực tiếp trong macro: tắt ngắt và vòng lặp vô hạn. Phân biệt macro, prototype và implementation; tránh định nghĩa hàm global nhiều lần trong header. |
| 21 | Gọi hàm assert riêng khác gì macro trực tiếp về breakpoint; đề có bắt buộc một cách không? | ChatGPT/Codex | Hai cách đều có thể dùng; hàm riêng thuận tiện đặt breakpoint chung, nhưng bài không bắt buộc tên vAssertCalled. |
| 22 | Include header cho __disable_irq trong config được không; tại sao phải truyền đường dẫn/STM32 macro qua target; stm32cubemx có phải INTERFACE không? | ChatGPT/Codex | Include stm32f1xx.h và nối freertos_config với stm32cubemx để kernel nhận include path CMSIS và STM32F103xB. |
| 23 | Config thuộc kernel hay firmware; source nhận đầu vào thế nào; thiếu macro có lỗi không; target có tự tìm source không? | ChatGPT/Codex | Hiểu chuỗi include FreeRTOS.h -> FreeRTOSConfig.h, thay macro trước compile, macro bắt buộc/mặc định và source phải được khai báo cho target. |
| 24 | params.h bắt buộc include hay chép số được; include trong config hay app; stack các task được đặt ở đâu? | ChatGPT/Codex | Giữ params.h trong app_main.c, dùng macro bài. Kernel config không cần tham số blink. Stack task truyền khi tạo task, không lấy mặc định từ stack Idle. |
| 25 | Xóa handler ở .c hay cả .h; hàm rỗng có gây trùng không; tại sao compile được nhưng link lỗi; gen CubeMX lại có sinh handler không? | ChatGPT/Codex | Bỏ cả định nghĩa handler CubeMX, có thể giữ prototype. Tắt Generate IRQ handler để không sinh lại; exception vẫn hoạt động qua handler kernel. |
| 26 | Phần cấu hình xong chưa, bước tiếp theo; các TODO trong app_main là gì; TODO1 có thể define không; được dùng HAL không? | ChatGPT/Codex | Define GPIO port/pin mask, viết task toggle rồi delay, tạo task và chạy scheduler. HAL ngoại vi được phép, CMSIS-RTOS wrapper bị cấm. |
| 27 | VS Code không gợi ý HAL/FreeRTOS; configure rồi vẫn không gợi ý; clangd không chạy và cảnh báo xung đột extension. | ChatGPT/Codex | Sửa đường dẫn app_main.c, cung cấp database compile; chuyển sang Microsoft C/C++ do STM32 clangd không khởi động. Sửa dấu phẩy trong workspace settings. |
| 28 | Phải viết prediction trước flash hay ghi số đo; sai số do đâu, startup có tính không; clock 8 MHz ảnh hưởng thế nào; khi nào sai lệch lớn? | ChatGPT/Codex | Prediction ghi dự đoán trước flash; startup tách khỏi chu kỳ ổn định. Phân biệt khoảng toggle 97 ms và chu kỳ HIGH-LOW 194 ms; xét tick, ISR, clock và jitter. |
| 29 | Câu 3: SysTick phục vụ bên nào, tick kernel có luôn tăng không; chưa chuyển HAL sang TIM4 thì vTaskDelay và uwTick ra sao? | ChatGPT/Codex | Theo handler/call chain để xác định bộ đếm được cập nhật. Nếu chỉ xử lý tick kernel, delay RTOS có thể chạy nhưng uwTick đứng yên, các hàm HAL phụ thuộc thời gian có vấn đề. |
| 30 | Câu 4: stack task dùng làm gì, tính chính xác hay ước lượng; bốn vùng RAM khác baremetal ra sao? | ChatGPT/Codex | 128 words = 512 byte, dự đoán đủ nhưng chưa chứng minh. Học file .su, high-water mark, stack task/ISR và heap_4 nằm trong .bss; tránh cộng RAM trùng. |
| 31 | Kiểm tra app_main; viết hook ở đâu, include gì; kernel có tự gọi không; vì sao overflow hook có tham số còn malloc hook không? | ChatGPT/Codex | Thêm hai hook vào app_main.c theo chữ ký kernel; dừng khi phát hiện lỗi. Stack hook nhận thông tin task, malloc hook không có tham số. |
| 32 | Có cần app_main.h để gọi từ main; tại sao không cần khai báo hook trong header; lỗi task.h chưa có FreeRTOS.h trước là gì? | ChatGPT/Codex | Nối app_main() sau khởi tạo GPIO. Hiểu header ứng dụng có thể chỉ khai báo app_main; nếu dùng task.h phải include FreeRTOS.h trước. |
| 33 | Tái hiện câu 5 nhưng không có lỗi; phân biệt chưa map handler và map rồi nhưng chưa bỏ handler CubeMX. | ChatGPT/Codex | AI kiểm tra symbol bản build còn tên handler gốc; build lại với config đã lưu để tái hiện trùng tên. Log multiple definition đã được người học lưu trong PREDICTION.md. |
| 34 | Soạn report trước khi flash, bỏ qua số đo thực tế. | ChatGPT/Codex | Nhận bản nháp Markdown về ba handler, HAL timebase và log linker; phần thực nghiệm để chưa đo. Chưa coi bản nháp là report đã hoàn tất. |
| 35 | AI_LOG có riêng từng ex không, phải ghi những gì; yêu cầu ghi lại log ex1/ex2. | ChatGPT/Codex | Dùng log riêng theo bài, tóm tắt các prompt liên quan và phần áp dụng; file này được tổng hợp từ lịch sử phiên hiện tại. |

## Các điểm đã được đính chính trong trao đổi

- Chia sẻ template là lựa chọn tổ chức, không phải khẳng định repo đã có cơ chế build hoàn chỉnh.
- 97 ms là khoảng giữa các lần toggle; chu kỳ HIGH–LOW đầy đủ dự kiến 194 ms.
- Không khẳng định dùng chung SysTick luôn làm vTaskDelay treo; phải xét handler cập nhật bộ đếm nào.
- Port GCC/ARM_CM3 của kernel hiện tại tự đặt PendSV/SysTick ở priority thấp nhất, không dùng trực tiếp configKERNEL_INTERRUPT_PRIORITY.
- Lỗi multiple definition xảy ra sau khi map tên mà chưa bỏ định nghĩa CubeMX, không phải chỉ vì chưa map handler.
- Build và kiểm tra source không thay thế đo timing/stack trên phần cứng.
