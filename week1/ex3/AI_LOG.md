# AI_LOG.md — Week 1 / Ex3

`student_id`: 2412108  
AI dùng: ChatGPT/Codex  
Ngày tổng hợp: 2026-10-17

Log được tổng hợp từ các lượt trao đổi trong phiên học này. Cột prompt là tóm tắt, không phải bản chép nguyên văn. Các lượt hỏi nối tiếp cùng nội dung được gom vào một mục, có liệt kê các vấn đề đã hỏi. Phần tích hợp FreeRTOS và thiết lập nền dùng lại được ghi thêm trong AI_LOG của ex1/ex2.

Người học tự sửa firmware, build, flash và thu dữ liệu trên board. AI đọc source, hướng dẫn, kiểm tra dữ liệu đã lưu và soạn các đoạn báo cáo trong chat. AI đã viết script phân tích cùng hướng dẫn sử dụng theo yêu cầu; không tự flash hoặc đọc trực tiếp giá trị RAM trên board. Các counter được ghi nhận từ ảnh/thông báo của người học. AI tạo log này và cập nhật .gitignore theo yêu cầu riêng, không kèm commit.

| # | Prompt (tóm tắt) | AI dùng | Mình giữ lại gì / sửa gì |
|---|---|---|---|
| 1 | Chuyển từ ex2 sang ex3; concept bài là gì; task toggle, tải giả và phần chờ diễn ra ở đâu? | ChatGPT/Codex | Một task toggle PA1, busy-wait tải biến đổi rồi block. So sánh delay tương đối và delay theo lịch, ở hai tick rate. |
| 2 | Chu kỳ 23 ms có quá lớn so với tải 600 us; tải biến đổi và cách delay ảnh hưởng như thế nào? | ChatGPT/Codex | Xét tải so với ranh giới tick, không chỉ tỷ lệ tải/chu kỳ. Tải dưới một tick có thể chưa tạo drift riêng rõ rệt cho vTaskDelay. |
| 3 | Nếu tick là 1 ms thì kernel biết thời điểm thập phân giữa hai tick bằng cách nào? | ChatGPT/Codex | Kernel dùng tick nguyên; DWT đo thời gian mức chu kỳ CPU. Không áp trực tiếp mô hình thời gian liên tục cho phép chờ bằng tick. |
| 4 | Danh sách project CMake tại sao chưa hiện ex3; config ex3 khác ex2 ở đâu? | ChatGPT/Codex | Kiểm tra sourceDirectory, reload VS Code, phân biệt project bằng đường dẫn. Bật INCLUDE_xTaskDelayUntil và giữ INCLUDE_vTaskDelay. |
| 5 | Có cần define bằng 1 cho mọi API; bắt đầu chuẩn bị source ex3 từ đâu? | ChatGPT/Codex | Phân biệt API tùy chọn với nhóm tính năng. CMake dùng app_main.c, periodic.c và dwt.c, bỏ tasks.c của ex2. |
| 6 | Đề quy định tải giả ra sao; kiểm tra DWT và header sau khi copy từ ex2. | ChatGPT/Codex | Dùng busy-wait DWT, tải trong [0, WORK_JITTER_US], seed cố định. Đổi tasks.h thành dwt.h, thêm stdint.h vào dwt.h. |
| 7 | Có nên gom toàn bộ include của source vào header; kiểm tra lại include. | ChatGPT/Codex | Header chỉ include phụ thuộc cần cho giao diện; source include phụ thuộc implementation. app_main.h chỉ cần prototype app_main. |
| 8 | next_work_us dùng làm gì; có phải random; xorshift32 cụ thể thế nào? | ChatGPT/Codex | Dùng ba bước XOR/dịch bit 13, 17, 5 và phép modulo WORK_JITTER_US + 1. Seed cố định đặt một lần, không reset trong mỗi lượt. |
| 9 | periodic_task viết như thế nào; #if khác if/else ở đâu; dùng if/else có được không? | ChatGPT/Codex | Giữ khung DELAY_MODE để chọn API lúc build. Init mốc delay-until ngoài vòng lặp; toggle, tạo tải, gọi delay trong vòng lặp. |
| 10 | Kiểm tra giá trị trả về xTaskDelayUntil để làm gì; ex3 có false không; trong ứng dụng khác false có ý nghĩa gì? | ChatGPT/Codex | pdTRUE là có block, pdFALSE là mốc đã đến/qua nên không block. Dự đoán ex3 tải nhẹ có 0 lần không block, nhưng phải kiểm chứng. |
| 11 | Khi không block thì task có lặp mãi không; hàm có cập nhật lịch không? | ChatGPT/Codex | Mốc vẫn tăng theo lịch. Task có thể bắt kịp và block ở lượt sau; nếu công việc luôn dài hơn chu kỳ thì có thể liên tục không block. |
| 12 | Yêu cầu đếm lần false nằm ở đâu; có phải tạo biến toàn cục không? | ChatGPT/Codex | REPORT mục 4 hỏi số lần trên SAMPLE_COUNT; dùng counter phạm vi file static. Biến toàn cục truy cập từ mọi file không bắt buộc. |
| 13 | Kiểm tra periodic.c nhiều lần; GPIO mask, tham số địa chỉ, hard-code chu kỳ và counter đúng chưa? | ChatGPT/Codex | Sửa GPIO thành 1U << PERIODIC_PIN, truyền &previous_wakeup_tick, dùng PERIOD_MS và SAMPLE_COUNT thay số chép trực tiếp. |
| 14 | Counter phải giới hạn ra sao; task có phải dừng ở 500 lượt; điều kiện <=500 hay <=501? | ChatGPT/Codex | Task được chạy liên tục. Đếm 500 lần delay đầu; 501 cạnh tạo 500 khoảng. Với sampleCount tăng trước delay, dùng <= SAMPLE_COUNT. |
| 15 | Vì sao đề xuất volatile; muốn tạm không dùng để xem debugger có đọc được không. | ChatGPT/Codex | Giữ counter periodic không volatile theo lựa chọn của người học; hiểu khả năng tối ưu hóa và giới hạn quan sát debugger. |
| 16 | Viết tiếp app_main và kiểm tra trước build/prediction. | ChatGPT/Codex | Tạo periodic_task stack 128 words, kiểm tra pdPASS. configMAX_PRIORITIES=2 và task priority=1 nên task có mức cao nhất. Stack chưa được xác nhận đầy đủ bằng high-water mark. |
| 17 | Làm bảng prediction; drift so với lịch nào; công thức có WORK_JITTER_US nghĩa là gì? | ChatGPT/Codex | Dùng lịch yêu cầu 23 ms để so drift. Phân biệt drift so với chu kỳ lượng tử hóa và sai lệch so với yêu cầu ứng dụng. |
| 18 | Bỏ qua độ phân giải tick là gì, tại sao lại bỏ qua; kiểm tra prediction đã điền. | ChatGPT/Codex | Loại cách áp trực tiếp công thức T=P+W cho trường hợp tải dưới tick. Ghi giả định tải cộng overhead/độ trễ chưa đẩy lời gọi sang tick tiếp theo. |
| 19 | Flash bằng gì, làm trong VS Code thế nào; kiểm tra launch.json, có phải sửa ELF path mỗi bài không? | ChatGPT/Codex | Dùng ST-Link/SWD và cấu hình debug phù hợp. Thảo luận dùng cmake.launchTargetPath thay đường dẫn ELF viết cứng. Người học tự flash. |
| 20 | Cấu hình 1 phải báo cáo gì; cần screenshot không; ảnh tổng quan và ảnh gắn mốc đã ổn chưa? | ChatGPT/Codex | Thu 500 khoảng, lưu CSV/capture và ảnh có thang thời gian. Ảnh minh họa tín hiệu, không thay thống kê từ CSV; nhãn ms làm tròn không chứng minh jitter nhỏ. |
| 21 | Có cần folder riêng; xuất .sr và CSV ở đâu; các tùy chọn export chọn thế nào? | ChatGPT/Codex | Lưu theo measurements/cfg1..cfg4. Save As cho .sr, Export Comma-separated values cho CSV; bật header và time column. |
| 22 | Kiểm tra dữ liệu vừa xuất, xuất lại rồi kiểm tra tiếp. | ChatGPT/Codex | CSV dedup đầu tiên có thời gian 0,1,2 us không khớp capture. AI đính chính hướng dẫn, yêu cầu tắt Dedup rows; giữ .sr gốc và xuất CSV đầy đủ từng mẫu. |
| 23 | Sau khi dữ liệu hợp lệ thì làm gì; nhờ viết script analyze. | ChatGPT/Codex | AI tạo tools/analyze_timing.py và tools/README.md. Script đọc theo luồng, lấy cả cạnh lên/xuống, chọn 501 cạnh để tính 500 khoảng, có tùy chọn sample rate và skip edges. |
| 24 | Vì sao script dùng chỉ số mẫu, kết quả jitter đúng 1 us; repo có sẵn tool không? | ChatGPT/Codex | CSV đầy đủ có cột thời gian bị làm tròn xuống tối đa 1 us; script dùng chỉ số mẫu/sample rate. Tool do AI viết theo yêu cầu, không có sẵn trong repo ban đầu. |
| 25 | Drift cfg1 âm và tích lũy lớn do đâu; clock khai báo khác clock thực như thế nào? | ChatGPT/Codex | Phân biệt sai lệch khoảng trung bình với tổng 500 khoảng. Sai lệch chung khoảng 0,084% có thể do clock MCU/timebase analyzer; chưa xác định nguyên nhân. |
| 26 | Sample rate trong ảnh là bao nhiêu; 20 giây có đúng 500 mẫu của bài không? | ChatGPT/Codex | Phân biệt 20 M mẫu analyzer với 500 khoảng task. Thu 20 M samples ở 1 MHz = 20 giây, đủ chọn 500 khoảng; không dùng nhầm cấu hình 20 kHz. |
| 27 | Thu cfg2 xong, kiểm tra kết quả; chạy script Python bị thiếu tham số. | ChatGPT/Codex | Xác nhận capture cfg2 1 MHz. Chạy script với --sample-rate 1000000; không truyền tick rate làm sample rate. |
| 28 | Đã làm cfg3, xem kết quả cfg4; kiểm tra vì hai bảng giống hệt nhau. | ChatGPT/Codex | AI so hash CSV, capture và disassembly ELF. Dữ liệu cfg3/cfg4 khác nhau nhưng thống kê 500 khoảng đầu trùng; ELF trên máy gọi delay-until 2 tick. Không coi đó là xác minh trực tiếp firmware trên board. |
| 29 | Vì sao kết quả dưới 23 ms, đặc biệt cfg1/cfg2? | ChatGPT/Codex | 100 Hz chuyển 23 ms thành 20 ms. Sai lệch tỷ lệ nhỏ còn thấy ở 1000 Hz, không thể quy hết cho lượng tử hóa tick hoặc API delay. |
| 30 | Xem noBlockCount như thế nào; sampleCount=2236 và noBlockCount=0 nghĩa gì; có cần kiểm tra cfg khác không? | ChatGPT/Codex | Người học đọc cfg4 từ debugger và báo cfg2 cũng 0. Ghi cfg2/cfg4 0/500; cfg1/cfg3 không áp dụng. Counter đầu chương trình chưa chắc trùng cửa sổ capture bắt đầu muộn. |
| 31 | Điền đường dẫn script, CSV và ảnh vào report. | ChatGPT/Codex | Nhận Markdown dùng đường dẫn tương đối tới tools và measurements/cfg1..cfg4. |
| 32 | Report phần 2 không dùng WORK_JITTER_US thì sao; dùng cfg nào; drift âm nghĩa gì và vì sao kết thúc sớm? | ChatGPT/Codex | Phân tích cfg1 và so thêm cfg2; 0<=W_i<=600 us, chưa chắc gây trượt tick. Drift âm là thời gian đo ngắn hơn lịch danh nghĩa, không tự chứng minh lỗi scheduling. |
| 33 | Phần 3 điền gì, chứng minh bằng số đo nghĩa là gì? | ChatGPT/Codex | So cfg1/cfg3: khoảng trung bình gần 23/20 ms. Tách sai lệch -1500 ms do lượng tử hóa khỏi sai lệch còn lại; không đồng nhất sai lệch chu kỳ với jitter. |
| 34 | Câu 6 yêu cầu gì, đo cụ thể như thế nào; dùng Idle task được không, đếm bằng cách nào? | ChatGPT/Codex | Dùng vApplicationIdleHook với configUSE_IDLE_HOOK=1, DWT xác định cửa sổ khoảng 8 triệu chu kỳ. Tạm tắt task periodic để so hai tick rate với cùng tải. |
| 35 | Đo một lần hay liên tục; đếm lần CPU vào Idle hay vòng lặp; biết một giây bằng cách nào? | ChatGPT/Codex | Mỗi reset đo một cửa sổ. idleLoopCount đếm lượt gọi hook, không phải context switch. DWT tiếp tục tăng trong ISR; hook ngừng đếm khi measurementDone=1. |
| 36 | Overhead là gì, vì sao chênh ít; đọc số trong report và kiểm tra sau cập nhật. | ChatGPT/Codex | Phân biệt overhead CPU tuyệt đối và mức giảm vòng Idle tương đối. Loại việc so hai lần khác tải; số cuối người học ghi là 111134 ở 100 Hz, 109455 ở 1000 Hz, giảm khoảng 1,51%. |
| 37 | Câu nguyên nhân sai lệch phải đổi nhanh/chậm thế nào? | ChatGPT/Codex | Cách diễn đạt rõ là clock thực nhanh hơn khai báo thì một tick ngắn hơn 1 ms thực. Đây vẫn là giả thuyết, không loại trừ analyzer. |
| 38 | Ex3 còn gì trước commit? | ChatGPT/Codex | AI chỉ ra cần khôi phục periodic, hoàn thiện giải thích/report, AI_LOG, check_api và xử lý CSV quá lớn. Không stage lẫn ex4. |
| 39 | Nhờ viết AI_LOG và cập nhật gitignore; hỏi rõ có bỏ CSV không; xác nhận tiếp tục. | ChatGPT/Codex | AI tạo log này và ignore CSV dưới measurements của riêng ex3. CSV trên máy không bị xóa; capture .sr, ảnh và script vẫn được theo dõi nếu được stage. |

## Kết quả đã kiểm tra từ dữ liệu đo

Đoạn phân tích mặc định lấy 501 cạnh toggle đầu trong mỗi CSV đầy đủ để có 500 khoảng, thời gian lấy từ chỉ số mẫu ở sample rate 1 MHz. Drift tính so với 500 x 23 ms = 11500 ms. Đây là số tính từ dữ liệu người học thu, không phải số đo do AI tự tạo.

| Cấu hình | Min (ms) | Max (ms) | Trung bình (ms) | Jitter (us) | Drift (ms) |
|---|---:|---:|---:|---:|---:|
| cfg1: vTaskDelay, 1000 Hz | 22.980 | 22.981 | 22.980614 | 1 | -9.693 |
| cfg2: xTaskDelayUntil, 1000 Hz | 22.980 | 22.981 | 22.980632 | 1 | -9.684 |
| cfg3: vTaskDelay, 100 Hz | 19.983 | 19.984 | 19.983148 | 1 | -1508.426 |
| cfg4: xTaskDelayUntil, 100 Hz | 19.983 | 19.984 | 19.983148 | 1 | -1508.426 |

AI đã chạy script trên CSV cfg1/cfg2; kiểm tra độc lập capture .sr cfg3/cfg4 và so hash hai CSV. Script cũng được kiểm tra với dữ liệu mẫu có kết quả biết trước: min/max/mean/drift, chọn đoạn bỏ cạnh đầu, chọn bit, từ chối thiếu cạnh và dữ liệu thưa có timestamp không khớp sample rate. Các kiểm tra này không xác minh độ chính xác tuyệt đối của clock MCU hoặc logic analyzer.

## Các đính chính và giới hạn cần giữ khi báo cáo

- Không dùng mô hình drift = tổng thời gian tải một cách trực tiếp cho tải nhỏ hơn một tick; phải xét lúc gọi delay có vượt ranh giới tick hay không.
- Không ép dữ liệu theo tiêu chí đề rằng hai API luôn phải có drift khác rõ rệt. Với tải 0–600 us hiện tại, hai API có thể cho kết quả rất gần nhau.
- Ở 100 Hz, delay-until có thể giữ đúng lịch 20 ms đã lượng tử hóa nhưng vẫn lệch so với yêu cầu 23 ms. Hai mốc drift này khác nhau.
- pdFALSE nghĩa là không block do mốc đến/qua, không tự khẳng định đã vi phạm deadline ứng dụng. Chỉ cfg2/cfg4 có phép kiểm tra này.
- Jitter 1 us nằm ở mức một mẫu của analyzer 1 MHz; chưa kết luận jitter thực của MCU chính xác bằng 1 us.
- Nguyên nhân sai lệch tỷ lệ khoảng 0,084% chưa được xác nhận. Clock MCU và timebase analyzer đều là khả năng cần kiểm chứng.
- Mức giảm vòng Idle khoảng 1,51% lấy 100 Hz làm mốc; không phải số phần trăm CPU tuyệt đối dành riêng cho tick ISR. Cần giữ cùng điều kiện đo giữa hai lần.
- AI đã thử chạy check_api.sh nhưng Git Bash bị môi trường công cụ chặn khởi động; chưa có kết quả PASS do AI xác nhận.
- Mỗi CSV khoảng 209 MB. Theo xác nhận của người học, .gitignore bỏ qua CSV lớn của ex3, không xóa dữ liệu trên máy. Đề vẫn yêu cầu nộp CSV: phải giải quyết bằng cách gửi riêng hoặc Git LFS; capture và script trong Git không tự thay thế yêu cầu nộp CSV.
- Các đề xuất sửa report đã được nêu trong chat; log này không có nghĩa mọi đề xuất đã được áp dụng vào REPORT.md.
