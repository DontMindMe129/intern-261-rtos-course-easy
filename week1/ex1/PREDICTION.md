# Dự đoán — Week 1 / Ex1

> **Commit file này TRƯỚC khi nạp chương trình lần đầu.** Report sẽ phải giải thích
> mọi chỗ dự đoán lệch so đo. Dự đoán sai không bị trừ điểm; dự đoán sau khi đo thì bị.

`student_id`: 2412108    Ngày/giờ commit: 14/10/2026 - 20:24

1. Chu kỳ LED tôi sẽ đo được: 97 ms (`BLINK_PERIOD_MS` = 97)
2. Sai số giữa chu kỳ đo được và `BLINK_PERIOD_MS`: 0 us. Vì sao?
   Lí do: theo em dự đoán thì trừ xung đầu tiên, các xung kế tiếp sau khi mcu đã chạy ổn định thì sẽ không lệnh nhiều bởi vì bài này không có nhiều task và ngắt
3. Nếu để HAL timebase ở lại SysTick, điều gì sẽ xảy ra khi `vTaskDelay()` được gọi?
   (chọn và giải thích: chạy bình thường / treo / tick sai nhịp / hard fault)  

   Trả lời: Nếu chỉ có 1 SysTick Handler, và với cấu hình hiện tại (vector table chỉ đến hàm Handler của kernel) nhưng chưa chuyển timebase của HAL sang Timer4 thì SySTick sẽ phục vụ tick của kernel nên hàm vTaskDelay() vẫn sẽ hoạt động bình thường, còn các hàm HAL có sử dụng uwTick sẽ không chạy vì SysTick Handler không gọi HAL_IncTick() dẫn đến uwTick không tăng.
4. `BRINGUP_STACK_WORDS` = 128 words = 128 * 4 = 512 bytes. Task này có đủ không? Vì sao?  

   Trả lời: theo em là đủ vì task led_blink không có mảng cục bộ lớn và không có đệ quy.

5. Trước khi map handler, linker sẽ báo lỗi gì (đoạn nguyên văn thông báo)  

   [build] C:/Embedded/intern-261-rtos-course-easy/third_party/FreeRTOS-Kernel/portable/GCC/ARM_CM3/port.c:230: multiple definition of `SVC_Handler'; CMakeFiles/firmware.dir/Core/Src/stm32f1xx_it.c.obj:C:/Embedded/intern-261-rtos-course-easy/week1/ex1/firmware/Core/Src/stm32f1xx_it.c:147: first defined here  

   [build] C:/Embedded/intern-261-rtos-course-easy/third_party/FreeRTOS-Kernel/portable/GCC/ARM_CM3/port.c:460: multiple definition of `PendSV_Handler'; CMakeFiles/firmware.dir/Core/Src/stm32f1xx_it.c.obj:C:/Embedded/intern-261-rtos-course-easy/week1/ex1/firmware/Core/Src/stm32f1xx_it.c:173: first defined here  

   [build] C:/Embedded/intern-261-rtos-course-easy/third_party/FreeRTOS-Kernel/portable/GCC/ARM_CM3/port.c:494: multiple definition of `SysTick_Handler'; CMakeFiles/firmware.dir/Core/Src/stm32f1xx_it.c.obj:C:/Embedded/intern-261-rtos-course-easy/week1/ex1/firmware/Core/Src/stm32f1xx_it.c:186: first defined here  
