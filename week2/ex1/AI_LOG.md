# AI_LOG — Week 2 / Ex1

`student_id`: 2412108 · AI: ChatGPT/Codex · Tổng hợp: 2026-10-25

Các prompt được tóm tắt theo chủ đề, không chép nguyên văn. Người học cấu hình
CubeMX, đấu dây, flash/debug và thu dữ liệu trên board. AI giải thích, viết/sửa
source theo yêu cầu, phân tích dump/log/ảnh và hỗ trợ prediction/report; không
trực tiếp điều khiển board.

## Nội dung hỗ trợ

| Prompt / chủ đề | Hỗ trợ và nội dung áp dụng |
|---|---|
| Queue khác mảng thế nào, handle chứa gì? | Giải thích control object, vùng chứa phần tử, copy payload, cấp phát động và handle. Kernel quản lý vị trí gửi/nhận, danh sách task chờ và block/unblock. |
| Tạo note, kiểm tra CubeMX và file config. | Tạo note local; kiểm tra CPU 72 MHz, I2C1 400 kHz, UART1; tích hợp kernel Cortex-M3/heap_4 qua CMake, cấu hình scheduler, heap và hooks. |
| Viết source từng bước, giữ TODO. | Dựng sensor → raw queue → filter → filtered queue → UART; priority 3/2/1. Giữ TODO, giải thích flow và lỗi trả về từng API. |
| Lọc trung bình trượt làm gì, sum/count/index ở đâu? | Dùng ba ring buffer và running sum: trừ mẫu cũ, thêm mẫu mới, chia số mẫu thực có khi khởi động. Không tạo lại toàn bộ mảng kết quả và không cộng lại cả cửa sổ mỗi lượt. |
| Có dùng GY-291/ADXL345 thay MPU6050 được không? | Thêm hai nhánh cảm biến, địa chỉ/ID/thanh ghi và thứ tự byte riêng. Lượt đo thực dùng ADXL345, địa chỉ 0x53, ODR nội bộ 400 Hz, host đọc mỗi 4 ms; không dùng FIFO/ngắt cảm biến. |
| Giải thích HAL_I2C_Mem_Read/Write, DLPF và cách kiểm tra lỗi. | Phân biệt địa chỉ thiết bị với địa chỉ thanh ghi, độ rộng địa chỉ thanh ghi với số byte dữ liệu; kiểm tra ID, trạng thái HAL và bỏ lần đọc lỗi. |
| Tính predict trước khi đo rồi commit. | Tính thời gian dây I2C/UART, giả định CPU xử lý kịp phần thời gian còn lại, dự đoán queue tối thiểu 1. Giải thích queue đầy, ba timeout và seq. Commit PREDICTION.md: c4dd892 trước các lượt đo. |
| Chưa có cảm biến thì chuẩn bị gì cho report? | Tạo profile, kế hoạch đo, logger RAM và script phân tích. Theo yêu cầu, rút logger còn time/raw_depth/filtered_depth mỗi 2 ms, 256 dòng; note và công cụ bổ sung giữ local qua gitignore. |
| Cấp nguồn, SDO và CS đấu thế nào? | Bổ sung dòng VCC bị thiếu trong hướng dẫn ban đầu; giải thích CS mức cao chọn I2C, SDO chọn 0x53/0x1D. Kiểm tra datasheet, không nối CS trực tiếp 5 V. |
| Xuất mảng RAM qua Debug Console thế nào? | Hướng dẫn dump binary memory. Tiền tố -exec không được adapter đang dùng chấp nhận; sửa thành lệnh GDB trực tiếp. Kiểm tra mỗi dump 3072 byte, 256 dòng, 0–510 ms. |
| Kiểm tra N16 và UART chậm S0. | N16 không drop, queue được quan sát ở mức 0. S0 filtered lần đầu thấy đầy 28 ms, raw gần rỗng, filter drop 94 tại cuối capture; UART có seq nhảy. Vẽ đồ thị từ dump thật. |
| Thử timeout hữu hạn/vô hạn. | SF: sensor/filter drop 62/25, raw đầy ở 76 ms. FI: drop 87/0, raw đầy ở 64 ms. SI: drop 0/0, seq liên tục nhưng backpressure làm sensor có thể đọc trễ. Các lỗi I2C/format/UART đều 0 trong ảnh. |
| Header báo INCLUDE_vTaskSuspend dù đã bật. | Thêm include FreeRTOS.h/params.h để experiment_config.h nhận cấu hình khi được phân tích riêng; build thành công, không tắt kiểm tra điều kiện FI/SI. |
| Kiểm chứng hai queue dài 1. | Thêm Q1, giữ UART 115200 và cửa sổ 16; không sửa params.h. Log 2348 mẫu seq 0–2347 liên tục, ảnh không lỗi/drop; chỉ kết luận trong điều kiện đã thử. |
| Câu 6 hỏi độ trễ nào, L16/N32 khác gì? | Làm rõ thời gian qua pipeline và độ trễ phản hồi lọc. DWT gắn timestamp vào mẫu ở L16/N32, từ trước I2C đến UART truyền xong, bỏ giai đoạn cửa sổ chưa đầy. TB 200 mẫu: 1904,737 và 1912,282 µs, tăng 7,545 µs. |
| Đo phản hồi bộ lọc bằng dữ liệu thật hay thử? | Sau khi thống nhất, thêm STEP16/STEP32: I2C vẫn chạy nhưng thay đầu vào lọc bằng 0 → 1000 tại seq 64, ngưỡng 500. Không cần lưu thêm toàn bộ lịch sử, chỉ chốt t0/t50 và seq. |
| Kiểm tra STEP16/STEP32. | Log khớp trung bình trượt, đạt 500 tại seq 71/79. DWT đến truyền xong UART: 29,196361/61,192458 ms, tăng 31,996097 ms, gần dự đoán 32 ms. Cả hai valid=done=1, không lỗi/drop. |
| Log L16 ghi nhầm N16. | Phát hiện log N16 gốc bị ghi đè, chép lượt mới sang L16; không tái tạo dữ liệu gốc. Người học thu lại N16, kiểm tra seq 0–1307 liên tục; dump/ảnh N16 cũ vẫn giữ nguyên. |
| Rà measurement, viết report ngắn và giữ câu hỏi. | Kiểm tra 10 lượt, tạo đồ thị/tổng hợp; điền draft, tách từng dự đoán ở mục 5. Phân biệt drop snapshot 510 ms với seq thiếu trên log dài hơn, giải thích backpressure và hai loại độ trễ. Chép sang REPORT.md và tổng hợp AI_LOG. |

## Kết quả và giới hạn

- Đã build các nhánh baseline, ADXL345, latency và STEP trước khi hướng dẫn
  đo; kết quả phần cứng lấy từ log/dump/ảnh do người học thu.
- Snapshot measurement_errors chốt ở cuối logger 510 ms. Pipeline vẫn chạy
  tiếp đến khi pause, nên không so số drop snapshot với seq thiếu toàn log
  như thể chúng cùng khoảng đo.
- Logger 2 ms có thể bỏ sót đỉnh queue ngắn; 28/64/76 ms là lần đầu quan sát
  thấy đầy, không phải phép đo chính xác cạnh đầy. Seq liên tục không chứng
  minh đọc đúng chu kỳ hoặc thu đủ mọi mẫu nội bộ ADXL345.
- Log N16 hiện là lượt thu lại riêng; dump/ảnh vẫn thuộc lượt N16 ban đầu.
  Các dòng UART cuối bị dở được loại khỏi thống kê dòng hoàn chỉnh.
- Pipeline latency gồm I2C, queue, lọc, định dạng và UART cùng overhead đo;
  không tính trễ nội bộ cảm biến hay hiển thị terminal. Chênh lệch 7,545 µs
  không được quy hết cho tính toán cửa sổ vì độ dài bản tin/lịch chạy cũng ảnh hưởng.
- STEP dùng dữ liệu thử, đo đáp ứng 50% đến truyền xong UART, không gọi trực
  tiếp là group delay. L16/N32 dùng ADXL345 thật. Dự đoán hai loại độ trễ được
  nêu trước các lượt đo tương ứng, bổ sung cho câu 6 ngoài prediction ban đầu.
- Source hiện chọn EX1_N16, SENSOR_ADXL345. Đổi EX1_PROFILE để tái tạo từng
  lượt, dùng Debug không có macro override. Không cần sửa params.h.
- Các phần bổ sung ban đầu giữ local theo yêu cầu. Khi hoàn tất, rà lại ignore:
  giữ source measurement/experiment_config, script phân tích và dữ liệu thật
  của 10 lượt để build và đối chiếu report. Note, draft, checklist, môi trường
  Python/cache và dữ liệu self-check vẫn được ignore.
