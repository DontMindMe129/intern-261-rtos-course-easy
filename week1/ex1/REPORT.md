# Report — Week 1 / Ex1

`student_id`: 2412108    Commit của PREDICTION.md: 7283630

## 1. Số đo
| Đại lượng | Giá trị |
|---|---|
| `BLINK_PERIOD_MS` (từ params.h) | 97 |
| Chu kỳ đo được — min |96.918 |
| Chu kỳ đo được — max |96.919 |
| Chu kỳ đo được — trung bình |96.918260 |
| Sai số trung bình vs `BLINK_PERIOD_MS` |-0.08174 |
| `uxTaskGetStackHighWaterMark()` của bringup_task | 98 words |

Ảnh PulseView — tick rate 1000 Hz:

![Dạng sóng PA0 khi tick rate là 1000 Hz](measurements/tick1000/Screenshot%202026-10-17%20192824.png)

## 2. Ba handler của kernel
| Tên trong port ARM_CM3 | Tên trong vector table | Nhiệm vụ |
|---|---|---|
| `vPortSVCHandler` | SVC_Handler | Khôi phục Context và chạy Task đầu tiên |
| `xPortPendSVHandler` | PendSV_Handler | Chuyển đổi giữa các Task |
| `xPortSysTickHandler` | SysTick_Handler | Cập nhật Tick, gọi PendSV nếu cần chuyển Task |

Log lỗi link trước khi sửa (dán nguyên văn):  

   [build] C:/Embedded/intern-261-rtos-course-easy/third_party/FreeRTOS-Kernel/portable/GCC/ARM_CM3/port.c:230: multiple definition of `SVC_Handler'; CMakeFiles/firmware.dir/Core/Src/stm32f1xx_it.c.obj:C:/Embedded/intern-261-rtos-course-easy/week1/ex1/firmware/Core/Src/stm32f1xx_it.c:147: first defined here  

   [build] C:/Embedded/intern-261-rtos-course-easy/third_party/FreeRTOS-Kernel/portable/GCC/ARM_CM3/port.c:460: multiple definition of `PendSV_Handler'; CMakeFiles/firmware.dir/Core/Src/stm32f1xx_it.c.obj:C:/Embedded/intern-261-rtos-course-easy/week1/ex1/firmware/Core/Src/stm32f1xx_it.c:173: first defined here  

   [build] C:/Embedded/intern-261-rtos-course-easy/third_party/FreeRTOS-Kernel/portable/GCC/ARM_CM3/port.c:494: multiple definition of `SysTick_Handler'; CMakeFiles/firmware.dir/Core/Src/stm32f1xx_it.c.obj:C:/Embedded/intern-261-rtos-course-easy/week1/ex1/firmware/Core/Src/stm32f1xx_it.c:186: first defined here  

Cách sửa: xóa 3 hàm Handler rỗng trong stm32f1xx_it.c

## 3. Tại sao HAL timebase phải sang TIM4
Phải trả lời được: `HAL_IncTick()` chạy ở đâu, `configKERNEL_INTERRUPT_PRIORITY`
ảnh hưởng gì đến nó, và `HAL_Delay()` / `vTaskDelay()` khác nhau chỗ nào.  

Trả lời:  
Vì kernel mặc định dùng SysTick làm nguồn đếm thời gian, nếu không đổi Timebase của HAL sang TIM4 mà lại đổi SysTickHandler sang Handler của kernel thì uwTick sẽ không được cập nhật => các hàm HAL liên quan không chạy được.  
Còn về configKERNEL_INTERRUPT_PRIORITY, kernel mặc định đặt SysTick_Handler ở priority thấp nhất nên nếu HAL vẫn dùng Systick thì việc cập nhật uwTick có thể bị chen ngan bởi các ngắt khác và nếu các ngắt khác lại đợi uwTick => kẹt.  
HAL_Delay() chờ bằng cách liên tục kiểm tra uwTick nên vẫn chiếm CPU khi được gọi trong task còn vTaskDelay() đưa task vào Blocked theo tick FreeRTOS, nhường CPU cho task khác.

## 4. Lệch so với dự đoán
Với mỗi mục trong `PREDICTION.md` lệch so đo: lệch bao nhiêu, và **nguyên nhân vật lý**
(không được trả lời "do sai số").  

Chu kỳ trung bình lệch so với dự đoán 0.08174ms. Nguyên nhân có thể là do thạch anh dao động với tần số không đúng chính xác 8MHz như đã khai báo.
## 5. Câu hỏi đánh đổi
`configTICK_RATE_HZ` = 1000 trong bài này. Dựa vào số đo của **bạn**, nếu hạ xuống
100 Hz thì chu kỳ LED và sai số thay đổi thế nào? Trả lời bằng con số, không bằng lý thuyết.  
| Đại lượng | Giá trị |
|---|---|
| Chu kỳ đo được — min |89.924000 |
| Chu kỳ đo được — max |89.925000 |
| Chu kỳ đo được — trung bình |89.924150 |
| Sai số trung bình vs `BLINK_PERIOD_MS` |-7.07585 |

Ảnh PulseView — tick rate 100 Hz:

![Dạng sóng PA0 khi tick rate là 100 Hz](measurements/tick100/Screenshot%202026-10-17%20193603.png)
