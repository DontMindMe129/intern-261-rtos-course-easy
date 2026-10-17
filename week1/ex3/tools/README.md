# Thống kê timing ex3

`analyze_timing.py` dùng Python 3 và thư viện chuẩn, không cần cài pip.

## Dữ liệu đầu vào

- Xuất CSV từ PulseView với **Time column** và **Output header** bật, **Dedup rows tắt**.
- Định dạng đang hỗ trợ: cột `microseconds` và `logic` (giá trị logic số nguyên thập phân).
- Mặc định phân tích bit 0 của cột logic, tương ứng D0 trong capture cfg1 hiện tại. Kiểm tra ánh xạ nếu thay đổi các kênh được xuất.
- Giữ capture `.sr` gốc để đối chiếu. Script không đọc hoặc thay đổi `.sr`.
- Truyền đúng sample rate của logic analyzer. Đây không phải clock CPU hoặc tick rate FreeRTOS.

Script đọc CSV theo luồng, không nạp toàn bộ file vào RAM. Nó lấy thời điểm bằng chỉ số mẫu chia sample rate, tránh lỗi làm tròn xuống 1 us đã quan sát trong cột thời gian xuất từ PulseView. Vì vậy bắt buộc đầu vào chứa **mọi mẫu**, không phải chỉ các dòng đổi mức. Script kiểm tra cột thời gian để bắt các sai lệch lớn, nhưng không thể tự phát hiện mọi lỗi xuất dữ liệu; đặc biệt không dùng CSV dedup cũ có thời gian bị đánh lại theo số dòng.

## Chạy từ thư mục gốc repo

```powershell
python week1/ex3/tools/analyze_timing.py week1/ex3/measurements/cfg1/data.csv --sample-rate 1000000
```

Đổi `cfg1` thành `cfg2`, `cfg3`, `cfg4` khi có dữ liệu tương ứng. Đổi `--sample-rate` nếu lần thu dùng tần số lấy mẫu khác.

Mặc định lấy **501 cạnh toggle liên tiếp đầu tiên**, gồm cả cạnh lên và xuống, để tạo **500 khoảng**. Không coi trạng thái ở dòng đầu CSV là một cạnh. Hai cạnh lên liên tiếp sẽ là chu kỳ HIGH–LOW đầy đủ, không phải khoảng task 23 ms của phép phân tích này.

Nếu cần bỏ đoạn khởi động, có thể chọn một phạm vi khác:

```powershell
python week1/ex3/tools/analyze_timing.py week1/ex3/measurements/cfg1/data.csv --sample-rate 1000000 --skip-edges 10
```

Ghi rõ số cạnh bỏ qua trong report. Counter `noBlockCount` hiện đếm 500 lần gọi đầu nên không tự tương ứng với một đoạn capture tùy chọn; cần đối chiếu/căn chỉnh riêng nếu dùng nó trong report.

Các lựa chọn khác:

- `--samples 500`: số khoảng cần thống kê, lấy từ `SAMPLE_COUNT` của bài.
- `--period-ms 23`: chu kỳ yêu cầu làm mốc tính drift, lấy từ `PERIOD_MS`.
- `--bit 0`: chọn bit trong giá trị logic được xuất.
- `--json`: in đầy đủ kết quả ở dạng JSON ra terminal.

Script không tự đọc params.h, không tự xác định mode firmware và không tự ghi report. Các giá trị mặc định 500 và 23 khớp bộ tham số ex3 hiện tại; đổi nếu bộ tham số thay đổi.

## Công thức

Với 501 cạnh tại chỉ số mẫu `s[0] .. s[500]`, sample rate `Fs`:

```text
dt[i] = (s[i+1] - s[i]) / Fs
min   = min(dt)
max   = max(dt)
mean  = (s[500] - s[0]) / (Fs * 500)
jitter = max - min
drift  = (s[500] - s[0]) / Fs - 500 * 0.023 s
```

Kết quả min/max/mean/drift được in bằng ms, jitter bằng us. Drift âm nghĩa là đoạn đo kết thúc sớm hơn lịch yêu cầu 23 ms/lượt.

Giữ mốc 23 ms cho cả bốn cấu hình để so sánh với yêu cầu ứng dụng. Ở tick 100 Hz, `pdMS_TO_TICKS(23)` mặc định thành 2 tick = 20 ms; lịch ổn định theo tick vẫn có thể có drift khoảng -1500 ms trên 500 khoảng so với lịch yêu cầu 23 ms. Nếu phân tích thêm drift so với lịch lượng tử hóa 20 ms, phải ghi rõ đây là mốc tham chiếu khác.

Độ phân giải 1 us ở 1 MHz không đồng nghĩa độ chính xác tuyệt đối 1 us; sai lệch timebase của thiết bị đo/MCU cần được xem xét riêng. Không suy ra nguyên nhân vật lý chỉ từ bảng thống kê.
