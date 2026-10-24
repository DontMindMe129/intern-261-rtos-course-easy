# Dự đoán — Week 2 / Ex4 (nháp)

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412108    `EVENT_BIT_COUNT` = 3  `SYNC_TIMEOUT_MS` = 80 ms

1. RAM event group chiếm (bytes) dự đoán: ______ . Dựa vào đâu (cấu trúc dữ
   liệu nào trong FreeRTOS)?

   **Trả lời:** Dự đoán object 28 byte; heap thực tế bị chiếm 40 byte.

   Nguyên văn trong `FreeRTOS-Kernel/event_groups.c`:

   ```c
    typedef struct EventGroupDef_t
    {
        EventBits_t uxEventBits;
        List_t xTasksWaitingForBits; /**< List of tasks waiting for a bit to be set. */

        #if ( configUSE_TRACE_FACILITY == 1 )
            UBaseType_t uxEventGroupNumber;
        #endif

        #if ( ( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
            uint8_t ucStaticallyAllocated; /**< Set to pdTRUE if the event group is statically allocated to ensure no attempt is made to free the memory. */
        #endif
    } EventGroup_t;
   ```

   Với cấu hình hiện tại:

   - `uxEventBits`: 4 byte, lưu bit sự kiện.
   - `xTasksWaitingForBits`: 20 byte = bộ đếm 4 + con trỏ duyệt 4 + nút mốc 12. Các nút task nằm trong TCB.
   - `uxEventGroupNumber`: 4 byte vì bật trace.
   - `ucStaticallyAllocated`: không có vì chỉ bật cấp phát động.

   ```text
   Object = 4 + 20 + 4 = 28 byte
   Heap   = 28 + 8 byte header, căn lên bội số 8 = 40 byte
   ```

   Handle `s_events` chiếm riêng 4 byte RAM tĩnh; tính cả handle là 44 byte.

2. RAM thêm khi dùng task notification cho 1 điểm đồng bộ: dự đoán = ______
   bytes. Vì sao?

   **Trả lời:** 0 byte. Giá trị và trạng thái notification đã nằm sẵn trong TCB của task nhận nên không cần cấp phát thêm object.

3. Dự đoán thời gian đánh thức (producer gọi → sync_task chạy) của notification
   so với event-group-bit: nhanh hơn / chậm hơn / xấp xỉ? Vì sao (nghĩ đến số
   lớp hàm bên dưới mỗi API)?

   **Trả lời:** Dự đoán notification nhanh hơn vì tác động trực tiếp đến task nhận qua handle. Event group còn phải duyệt danh sách chờ và kiểm tra điều kiện bit.

4. Nếu `sync_task` bị timeout (`SYNC_TIMEOUT_MS` hết hạn) vì một producer
   không set bit kịp, `xEventGroupWaitBits` trả về gì? Làm sao phân biệt được
   "đủ bit" với "timeout"?

   **Trả lời:** Trả về giá trị các bit sự kiện khi kết thúc chờ, ta phải tự kiểm tra kiểm tra:

   ```c
   (bits & wait_mask) == wait_mask
   ```

   - Đúng: đủ bit  
   - Sai: chưa đủ bit, kết thúc chờ do timeout. Khi chưa đủ điều kiện, các bit đã có không bị xóa bởi clear-on-exit.

5. Nêu trước một tình huống bạn nghĩ **không thể** thay toàn bộ event group
   bằng notification — sẽ kiểm chứng lại ở REPORT.md.

   **Trả lời:** Khi nhiều task cùng chờ một sự kiện: set một bit event group có thể đánh thức nhiều task thỏa điều kiện. Notification chỉ gửi đến một task cụ thể, nên không thay trực tiếp được cơ chế này.

