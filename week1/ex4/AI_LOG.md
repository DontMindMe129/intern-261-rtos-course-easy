# AI_LOG.md — Week 1 / Ex4

`student_id`: 2412108  
AI dùng: ChatGPT/Codex  
Ngày tổng hợp: 2026-10-20

Log tổng hợp các trao đổi ex4 trong ngày 2026-10-19–2026-10-20. Prompt được tóm tắt; các câu hỏi nối tiếp cùng chủ đề được gom lại, không phải bản chép nguyên văn. Người học tự sửa source, build, flash, nối UART và thu log. AI đọc source, giải thích, phân tích file đo, điền số liệu report và thực hiện các thao tác file/Git khi người học yêu cầu.

## Nội dung hỗ trợ

| # | Prompt / chủ đề tóm tắt | Hỗ trợ của AI và nội dung giữ lại |
|---|---|---|
| 1 | Bắt đầu ex4 từ đâu, mục tiêu bài là gì? | Đọc README/RUBRIC; xác định ba phần: ngân sách RAM từ map, heap/HWM lúc chạy, thử tràn stack và chọn lại stack. Chưa flash trước khi commit prediction. |
| 2 | Kiểm tra CMake sau khi copy cấu hình từ bài cũ. | Chỉ ra cần link executable với freertos_kernel; freertos_config phải truyền include path app vì FreeRTOSConfig.h include params.h để lấy HEAP_SIZE_BYTES. Đối chiếu ex1–ex3: các config đó không include params.h. |
| 3 | Cấu hình dynamic/static và timer task như thế nào? | Hướng dẫn bật cả hai kiểu cấp phát, software timers; timer priority 1, queue length 5, stack 128 words. Giữ clock 8 MHz, tick 1000 Hz, heap 9216 bytes và kiểm tra tràn mức 2. |
| 4 | Callback là gì; timer callback khác callback cấp bộ nhớ thế nào? | Phân biệt software timer, timer task và vApplicationGetTimerTaskMemory. Ex4 chưa tạo software timer ứng dụng; timer task tồn tại để đo bộ nhớ và chủ yếu chờ lệnh. |
| 5 | Hai callback Idle/timer được gọi khi nào, truyền tham số gì? | Kernel gọi trong lúc khởi động scheduler để nhận địa chỉ TCB, stack và số words. Giải thích output parameters, con trỏ kép và phép gán qua con trỏ. |
| 6 | Vì sao bài cũ không có callback bộ nhớ Idle? | Phân biệt Idle dynamic ở cấu hình cũ và Idle dùng bộ nhớ static khi bật static allocation trong cấu hình này; không nhầm với vApplicationIdleHook. |
| 7 | Cấp phát tĩnh cho Idle/timer ra sao; bỏ static được không? | Hướng dẫn StaticTask_t và mảng StackType_t riêng cho từng task. Phân biệt static ở phạm vi file (liên kết nội bộ) với static trong hàm (thời gian tồn tại); không trả buffer tự động hết hạn sau callback. |
| 8 | Tạo dyn_task/stat_task; shared_work làm gì? | Cả hai dùng shared_work với delay 50 ms trong bản normal, stack 160 words mỗi task. Dynamic lấy TCB/stack từ heap; static dùng statTaskTCB/statTaskStack. |
| 9 | xTaskCreateStatic trả NULL khi nào; handle có phải địa chỉ TCB? | Giải thích kiểu trả về, assert con trỏ buffer, không xin heap cho TCB/stack static. Trong kernel hiện tại handle tham chiếu TCB; dùng handle API trả về thay vì tự ép kiểu. |
| 10 | Khi chuyển task có phải chép stack đi; vì sao không return khỏi task? | Stack task giữ trong RAM; context CPU được lưu/khôi phục và vị trí stack giữ trong TCB. Task return đi tới lỗi của port; muốn tự xóa phải dùng vTaskDelete khi API được bật. |
| 11 | Tạo task báo cáo RAM; ba task cùng priority có sao không? | Tạo ram_report với stack ban đầu 256 words, priority 1. Các task có block, chưa có tranh chấp UART giữa các task; timer priority 1, Idle priority 0. |
| 12 | uart_print, print_stack, snprintf và size_t dùng làm gì? | Tách truyền chuỗi UART khỏi đọc/định dạng HWM. Giải thích sizeof, words/bytes, kiểm tra snprintf bị cắt và kiểm tra trạng thái HAL. size_t phù hợp kiểu trả về API heap. |
| 13 | Polling UART có phải yêu cầu bài; có time slicing không? | Polling được chọn cho đơn giản, không phải bài cấm interrupt/DMA. Polling giữ task muốn chạy nhưng không tắt ngắt; time slicing/preemption vẫn hoạt động. USART1 thực tế 9600 baud, 8N1. |
| 14 | Log những số nào; NULL khi đo ram_report có dùng được không? | Log heap hiện tại/thấp nhất và HWM dyn, stat, ram_report, Idle, timer; in words và bytes. NULL trong uxTaskGetStackHighWaterMark chỉ task đang gọi. |
| 15 | Source đã xong chưa; nối main và extern thế nào? | Hướng dẫn gọi app_main sau GPIO/UART init. Giải thích extern cho biến dùng khác file, khai báo hàm và biến có quy tắc khác nhau. |
| 16 | Viết hook lỗi; vì sao không dùng helper HAL UART sẵn có? | Hướng dẫn hai thông báo cố định, tắt ngắt rồi polling trực tiếp TXE/TC với số vòng chờ hữu hạn. Không dùng snprintf; không phụ thuộc HAL tick có thể ngừng khi tắt ngắt. Hai hook vẫn không bảo đảm báo được nếu bộ nhớ hỏng nặng. |
| 17 | Build thành công; đọc file map từ đâu? | Người học báo build thành công. AI đọc map, giải thích MEMORY/SECTIONS trong linker script, map đầu ra, archive members, discarded sections, LOAD/GROUP, symbol, alignment và phần đóng góp của object. |
| 18 | Map chỉ biết static hay có cả RAM/Flash; vì sao libfreertos_kernel.a? | Giải thích archive .a, tên target với tiền tố lib, input/output sections và thư viện INTERFACE/OBJECT. Map mô tả bố trí khi link, không liệt kê block dynamic được cấp lúc chạy. |
| 19 | data/bss/ucHeap đọc và cộng ngân sách thế nào? | Tách bss tổng 12008 thành ucHeap 9216 và phần còn lại 2792; data 96, heap C tối thiểu 512, MSP tối thiểu 1024, phần còn lại 6840. Tổng 20480; không cộng trùng ucHeap hoặc stack static. |
| 20 | Stack MSP tối thiểu là gì, ai chừa, sau scheduler còn dùng không? | Đối chiếu _Min_Stack_Size, _estack và startup/port. Đây là ngân sách linker, không giới hạn bảo vệ phần cứng; MSP tiếp tục dùng cho handler, task dùng PSP. Phân biệt bỏ stack frame với giải phóng toàn bộ vùng nhớ. |
| 21 | Biến global/static/local được tạo khi nào? | Phân biệt vùng dành khi link và startup khởi tạo với frame local trên stack khi chạy; gán biến global trong task không tạo thêm vùng nhớ. |
| 22 | Điền report trước prediction có vấn đề không; prediction dựa trên gì? | Ngân sách map có thể biết trước flash; heap/HWM là dự đoán runtime. Chấp nhận dự đoán đơn giản 7400 = 9216 − 1816, rồi đối chiếu overhead thực tế; không sửa prediction theo số đo về sau. |
| 23 | Có bao nhiêu task, mỗi task bao nhiêu bytes? | Normal có 5 task; TCB bản build 76 bytes. Dyn/stat stack 640 bytes, report 1024, Idle/timer mỗi stack 512. Chỉ dyn/report cấp TCB/stack dynamic. |
| 24 | Dự đoán HWM, hog và stack nhỏ nhất dựa vào đâu? | Giải thích HWM là words trống ít nhất theo phép đo; dùng giả định, không khẳng định tính chính xác từ biến cục bộ. Người học dự đoán dyn/stat 140, Idle 108, timer 100; stack có hog 212 words. |
| 25 | Biên ít nhất 32 words có yêu cầu không, phải giải thích gì? | Trích README yêu cầu biên >= 32 words; phân biệt ngưỡng của bài với bảo đảm chung của RTOS. Cần giải thích lựa chọn và kiểm chứng HWM. |
| 26 | Kiểm prediction, commit và điền hash. | Rà đủ sáu câu, lỗi cột bảng được người học sửa. Theo yêu cầu, AI commit riêng prediction ddaeeb3 rồi điền hash vào REPORT.md. Commit này được tạo trước lượt flash đầu được trao đổi trong phiên. |
| 27 | Hercules không có log; nối và debug thế nào? | Kiểm tra source PA9 TX, USART1 9600, TIM4 HAL timebase, handler kernel và đường gọi task. Hướng dẫn breakpoint tại HAL_UART_Transmit, Continue và kiểm tra RX/GND/COM. Người học sau đó báo nhận được log. |
| 28 | Xuất log Hercules ra sao? | Tra tài liệu menu Log to File/Log Enable, hướng dẫn lưu raw UART. Đọc các lượt log từ file người học lưu thay vì tạo số đo giả. |
| 29 | Kiểm log normal, lưu/commit map, chỉ điền số liệu report. | Xác nhận heap 7344, các HWM; sao chép nguyên bản map sang measurements/normal/firmware.map, kiểm tra hash bản sao, commit riêng 8e731bc theo yêu cầu và điền report. Điền số liệu/log nhưng không tự viết thay toàn bộ phần giải thích. |
| 30 | Vì sao volatile và vòng for cho hog; task static có lỗi gì? | Hướng dẫn volatile để giữ thao tác ghi khi tối ưu, for ghi từng byte. Static task vẫn có thể tràn stack; malloc-failed liên quan thao tác cấp heap. Giải thích thời điểm kernel phát hiện từng lỗi. |
| 31 | Thử tràn chỉ dyn, kiểm source. | Tạo dyn_hog_work riêng, giữ stat chạy shared_work. Dyn stack 160 words, hog 640 bytes. AI đính chính mẫu ban đầu 0xA5 thành 0x5A để không trùng mẫu điền stack của kernel. Người học tự sửa và chạy. |
| 32 | Kiểm log overflow. | Đọc measurements/overflow/uart_log.log có FAULT: stack overflow. Không suy từ việc source có hook rằng hook chắc chắn đã chạy. |
| 33 | Thử stack212 rồi stack220, HWM có đủ không? | Quan sát lần 212 words còn 25, chưa đạt 32. Suy mức dùng khoảng 187 words, đề nghị 220; log stack220 xác nhận HWM 33, heap 7104. Chỉ gọi 220 là mức đã kiểm chứng, chưa chứng minh nhỏ nhất tuyệt đối. |
| 34 | Có cần thêm đo cho câu 8; dynamic/static khác gì khi chỉ thêm 128 words? | Nêu hai nguồn RAM: heap còn trống và bss ngoài heap; static không miễn phí. Chọn dynamic vì còn đủ heap, thử một phương án theo đề. |
| 35 | Handle có cần không; sao in stack/heap bằng các helper khác nhau? | Handle dyn/stat được dùng để lấy HWM. Add task có handle riêng để log; heap không có task handle. print_stack đọc/định dạng stack, uart_print chỉ gửi chuỗi. |
| 36 | Kiểm source thêm task và sửa lỗi khai báo trùng result. | Phát hiện khai báo BaseType_t result hai lần cùng phạm vi; người học sửa thành khai báo một lần và gán lại, kiểm tra kết quả ngay sau mỗi lần tạo. Add task stack 128 words, priority 1, shared_work. |
| 37 | Kiểm log thêm task, điền số liệu report. | Xác nhận heap 7104 → 6504, giảm 600; HWM add 103, dyn 33. AI điền bảng số, log overflow, thử stack và đối chiếu prediction theo yêu cầu; giữ phần giải thích cho người học. |
| 38 | Hoàn thiện từng phần report, nguyên nhân dự đoán lệch. | Thảo luận dynamic/static, UART lỗi không phụ thuộc tick, mức stack đã kiểm chứng và giả định HWM. Giải thích 56 bytes chênh heap; không gán nguyên nhân chi tiết không có phép đo. |
| 39 | Rà report và chạy check_api. | Đối chiếu số với log, lưu ý thiếu log212 và chưa chứng minh mức nhỏ nhất tuyệt đối. Người học báo PASS cho lệnh giới hạn app và Core; AI không nhận là tự chạy hoặc là toàn repo PASS. |
| 40 | Rà toàn bộ ex4 và cập nhật AI log. | Rà source/config/CMake/main/handler, report, bằng chứng và commit; kiểm tổng ngân sách, liên kết file và nhóm số trong log. Chỉ tạo AI_LOG.md theo yêu cầu, không đổi source/report hoặc commit/push thêm. |

## Bằng chứng và kết quả

| Cấu hình | Bằng chứng | Kết quả đã xác nhận |
|---|---|---|
| Normal: dyn/stat cùng shared_work, mỗi task stack 160 words; report 256; chưa có add_task | [UART normal](measurements/normal/uart_log.log), [map normal](measurements/normal/firmware.map) | Heap hiện tại/thấp nhất 7344 B; HWM dyn/stat 135, report thấp nhất 70, Idle thấp nhất quan sát 109, timer 84 words. |
| Overflow: dyn_hog_work, stack 160 words, hog 640 B ghi 0x5A | [UART overflow](measurements/overflow/uart_log.log) | Có thông báo FAULT: stack overflow. |
| Có hog, stack212 | Quan sát UART trong phiên; không còn file log riêng trong bộ bằng chứng hiện tại | HWM dyn 25 words; heap đã quan sát 7136 B. Không đạt biên 32 words. |
| Có hog, stack220; chưa thêm task | [UART stack220](measurements/stack220/uart_log.log) | Heap 7104 B; HWM dyn 33 words = 132 B. Có đoạn đầu file bị cắt/lỗi ký tự; các lượt báo cáo đầy đủ phía sau dùng để đọc số, file gốc được giữ nguyên. |
| Thêm task dynamic 128 words; dyn vẫn có hog và stack220 | [UART thêm task](measurements/add_task/uart_log.log) | Heap 6504 B, vẫn >= 4096; HWM add 103, dyn 33 words. Giảm 600 B so với stack220. |

Normal: phần heap không còn tự do là 9216 − 7344 = 1872 B: TCB/stack hai task dynamic 1816 B, metadata/căn chỉnh bốn block 40 B, phần khởi tạo/căn chỉnh heap 16 B. Task thêm: stack 512 + TCB 76 + overhead 12 = 600 B. Phương án static chỉ được phân tích, chưa thực hiện: TCB/stack 588 B, chưa kể padding bố trí.

## Đính chính và giới hạn

- AI ban đầu nói timer queue lấy từ heap. Sau khi đọc timers.c và map thực tế, đã đính chính: với configSUPPORT_STATIC_ALLOCATION = 1 của kernel này, timer queue dùng xQueueCreateStatic, nằm trong bss và không trừ tiếp vào heap FreeRTOS.
- AI ban đầu đưa mẫu ghi hog bằng 0xA5; đã sửa trước lần thử được xác nhận thành 0x5A vì kernel dùng 0xA5 đánh dấu stack. Không dùng mẫu trùng để suy mức stack đã ghi.
- HWM là kiểm tra vùng còn mẫu điền stack; log Idle có dao động 109–110 words, nên report dùng giá trị thấp nhất quan sát 109. Không diễn giải đây là phép đo stack tuyệt đối chính xác cho mọi đường chạy.
- Source hiện tại là cấu hình câu 8: dyn có hog, stack 220 words, thêm task 128 words và log sáu task. Bảng map/report normal là bản baseline trước các thay đổi này; không khẳng định map normal tái hiện mọi địa chỉ của bản build cuối.
- Hai hook đều có code báo lỗi ra UART; chỉ hook stack overflow có bằng chứng thực chạy trong bộ log. Chưa có thử nghiệm malloc-failed riêng, đề không yêu cầu gây lỗi này riêng.
- Hook overflow hiện gửi nhãn lỗi cố định, không đọc tên task/handle có thể đã bị hỏng. Không dùng kết quả đó để khẳng định mọi trường hợp tràn đều phát hiện được.
- Lần thử212 không có raw log riêng. Mức220 đạt biên >=32 trong các lượt quan sát; chưa thử219 và không khẳng định là nhỏ nhất tuyệt đối hoặc an toàn với mọi thay đổi compiler/code.
- Build/flash và UART do người học thực hiện. AI đọc file kết quả; lượt rà này không tự build, flash hay đo phần cứng.
- check_api: người học báo PASS sau `bash tools/check_api.sh week1/ex4/app week1/ex4/firmware/Core`. Không có transcript kiểm tra lưu riêng, không suy rộng thành kiểm toàn repo/Drivers.
- Prediction commit: `ddaeeb3`; map baseline commit: `8e731bc`. Hai thao tác commit được AI thực hiện theo yêu cầu rõ ràng; report được điền hash sau đó. Chưa push hai commit này trong phần làm ex4.
- Những thay đổi ex1, ex2 hoặc week2 không nằm trong lượt cập nhật này. AI không cập nhật memory hệ thống.
