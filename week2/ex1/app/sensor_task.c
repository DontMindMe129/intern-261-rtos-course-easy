/* I2C polling: MPU6050 hoặc GY-291/ADXL345 -> queue_raw.
 * Chọn SENSOR_TYPE bên dưới hoặc qua compiler define.
 * Nguồn: TDK MPU-6000/6050 Register Map; Analog Devices ADXL345 Rev. G.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "main.h"
#include "params.h"
#include "pipeline.h"

extern I2C_HandleTypeDef hi2c1;

#define SENSOR_MPU6050  1
#define SENSOR_ADXL345  2

#ifndef SENSOR_TYPE
#define SENSOR_TYPE SENSOR_MPU6050
#endif

/* Địa chỉ 7 bit, đổi theo mức chân AD0 / SDO thực tế. */
#define MPU6050_ADDR_7BIT 0x68U /* AD0=0; AD0=1 -> 0x69. */
#define ADXL345_ADDR_7BIT 0x53U /* SDO=0; SDO=1 -> 0x1D; CS nối 3.3 V. */

/* HAL timeout tính bằng ms; queue timeout tính bằng tick. */
#define SENSOR_I2C_TIMEOUT_MS 20U
#define SENSOR_SEND_TIMEOUT EX1_SENSOR_TIMEOUT_TICKS

/* Xem bằng debugger. seq đếm lần thử đọc, không phải bộ đếm nội bộ cảm biến. */
volatile uint32_t sensor_read_errors = 0;
volatile uint32_t sensor_queue_drops = 0;

#if SENSOR_TYPE == SENSOR_MPU6050
#define SENSOR_ADDR       (MPU6050_ADDR_7BIT << 1)
#define SENSOR_ID_REG     0x75U
#define SENSOR_ID_VALUE   0x68U
#define SENSOR_DATA_REG   0x3BU /* ACCEL_XOUT_H, byte cao trước. */
#elif SENSOR_TYPE == SENSOR_ADXL345
#define SENSOR_ADDR       (ADXL345_ADDR_7BIT << 1)
#define SENSOR_ID_REG     0x00U
#define SENSOR_ID_VALUE   0xE5U
#define SENSOR_DATA_REG   0x32U /* DATAX0, byte thấp trước. */
#else
#error "SENSOR_TYPE must be SENSOR_MPU6050 or SENSOR_ADXL345"
#endif

static HAL_StatusTypeDef sensor_read(uint8_t reg, uint8_t *data, uint16_t size)
{
    return HAL_I2C_Mem_Read(&hi2c1, SENSOR_ADDR, reg, I2C_MEMADD_SIZE_8BIT,
                            data, size, SENSOR_I2C_TIMEOUT_MS);
}

static void sensor_write(uint8_t reg, uint8_t value)
{
    if (HAL_I2C_Mem_Write(&hi2c1, SENSOR_ADDR, reg, I2C_MEMADD_SIZE_8BIT,
                          &value, 1, SENSOR_I2C_TIMEOUT_MS) != HAL_OK) {
        Error_Handler();
    }
}

static void sensor_init(void)
{
    uint8_t id = 0;

    vTaskDelay(pdMS_TO_TICKS(100)); /* Chờ nguồn ổn định, nhường CPU. */
    if (sensor_read(SENSOR_ID_REG, &id, 1) != HAL_OK || id != SENSOR_ID_VALUE) {
        Error_Handler();
    }//kiem tra id cam bien

#if SENSOR_TYPE == SENSOR_MPU6050
    /* DLPF bật -> base rate 1 kHz; divider theo SAMPLE_PERIOD_MS. */
    configASSERT(SAMPLE_PERIOD_MS >= 1 && SAMPLE_PERIOD_MS <= 256);
    sensor_write(0x6BU, 0x80U); /* PWR_MGMT_1: reset. */
    vTaskDelay(pdMS_TO_TICKS(100));
    sensor_write(0x6BU, 0x01U); /* Thoát sleep, clock PLL gyro X. */
    vTaskDelay(pdMS_TO_TICKS(30));
    sensor_write(0x6CU, 0x00U); /* PWR_MGMT_2: bật các trục. */
    sensor_write(0x1AU, 0x03U); /* CONFIG: DLPF, accel bandwidth 44 Hz. */
    sensor_write(0x19U, (uint8_t)(SAMPLE_PERIOD_MS - 1U)); /* 4 ms -> 250 Hz. */
    sensor_write(0x1CU, 0x00U); /* ACCEL_CONFIG: ±2 g, 16384 LSB/g. */
    sensor_write(0x38U, 0x00U); /* INT_ENABLE: không dùng ngắt cảm biến. */
    sensor_write(0x23U, 0x00U); /* Không đưa mẫu vào FIFO. */
    sensor_write(0x6AU, 0x00U); /* USER_CTRL: tắt FIFO/I2C master nội bộ. */
#else
    /* Không có ODR 250 Hz: dùng 400 Hz, host đọc mỗi 4 ms.
     * Bỏ qua một số mẫu nội bộ; seq chỉ theo dõi pipeline của host.
     * Đổi I2C1 sang 400 kHz trong CubeMX trước khi chạy lựa chọn này.
     */
    if (hi2c1.Init.ClockSpeed != 400000U) {
        Error_Handler();
    }
    sensor_write(0x2DU, 0x00U); /* POWER_CTL: standby để cấu hình. */
    sensor_write(0x2EU, 0x00U); /* INT_ENABLE: không dùng ngắt cảm biến. */
    sensor_write(0x38U, 0x00U); /* FIFO_CTL: bypass. */
    sensor_write(0x31U, 0x08U); /* DATA_FORMAT: full resolution, ±2 g,
                               * right justified, khoảng 256 LSB/g. */
    sensor_write(0x2CU, 0x0CU); /* BW_RATE: normal power, ODR 400 Hz. */
    sensor_write(0x2DU, 0x08U); /* POWER_CTL: measurement mode. */
#endif
    vTaskDelay(pdMS_TO_TICKS(100)); /* Chờ bộ lọc/cảm biến ổn định. */
}

/* Ghép byte thành signed 16 bit, không phụ thuộc endian của MCU. */
static int16_t decode_axis(uint8_t first, uint8_t second)
{
#if SENSOR_TYPE == SENSOR_MPU6050
    uint16_t bits = ((uint16_t)first << 8) | second;
#else
    uint16_t bits = ((uint16_t)second << 8) | first;
#endif
    int32_t value = bits;
    if (bits & 0x8000U) {
        value -= 65536;
    }
    return (int16_t)value;
}

void sensor_task(void *argument)
{
    (void)argument;
    uint32_t seq = 0; // stt cua 1 sample
    const TickType_t period = pdMS_TO_TICKS(SAMPLE_PERIOD_MS);

    configASSERT(queue_raw != NULL);
    configASSERT(period > 0);
    sensor_init();
    measurement_begin();

    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        uint8_t data[6];
        raw_sample_t s = {0};
        s.seq = seq++;

        /* Burst read 6 byte cho ba trục. Không gửi mẫu nếu đọc lỗi. */
        if (sensor_read(SENSOR_DATA_REG, data, sizeof(data)) == HAL_OK) {
            s.ax = decode_axis(data[0], data[1]);
            s.ay = decode_axis(data[2], data[3]);
            s.az = decode_axis(data[4], data[5]);

            if (xQueueSend(queue_raw, &s, SENSOR_SEND_TIMEOUT) != pdPASS) {
                sensor_queue_drops++;
            }
        } else {
            sensor_read_errors++;
        }

        /* Lỗi I2C có thể làm vượt chu kỳ; delay không bảo đảm deadline. */
        vTaskDelayUntil(&last_wake, period);
    }
}
