#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>
#include <stdbool.h>

#pragma pack(push, 1)
typedef struct {
    uint8_t header[4];   // 'T','C','S','Y'
    uint8_t type;
    uint8_t ver;
    uint16_t len;
    uint8_t radar_id;
    uint16_t target_flag;
    uint8_t status;
    int16_t x;
    int16_t y;
    int16_t z;
    uint8_t reserved;
    uint8_t checksum;
} r60_frame_t;
#pragma pack(pop)


#define UART_PORT   UART_NUM_1
#define TX_PIN    16
#define RX_PIN    17
#define BUF_SIZE    1024

static const char *TAG = "R60ATR2";

/* PROTOTIP*/
void R60_task(void *arg);

void R60_uart_init(void) {
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0);
}


bool r60_parse_frame(uint8_t *buf, int len, r60_frame_t *out)
{
    if (len < sizeof(r60_frame_t)) return false;

    if (buf[0] != 0x54 || buf[1] != 0x43 ||
        buf[2] != 0x53 || buf[3] != 0x59) {
        return false;
    }

    memcpy(out, buf, sizeof(r60_frame_t));
    return true;
}



void R60_task(void *arg)
{
    uint8_t data[BUF_SIZE];
    r60_frame_t frame;

    ESP_LOGI(TAG, "R60 task startan");

    while (1) {
        int len = uart_read_bytes(UART_PORT, data, BUF_SIZE, pdMS_TO_TICKS(1000));
        if (len <= 0) continue;

        for (int i = 0; i <= len - sizeof(r60_frame_t); i++) {

            if (r60_parse_frame(&data[i], len - i, &frame)) {

                bool presence = (frame.status & 0x80);

                ESP_LOGI(TAG,
                    "PRESENCE=%s | X=%d | Y=%d",
                    presence ? "YES" : "NO",
                    frame.x,
                    frame.y
                );

                i += sizeof(r60_frame_t) - 1;
            }
        }
    }
}


void app_main(void) {
    R60_uart_init();
    xTaskCreate(R60_task, "R60_read_task", 4096, NULL, 10, NULL);
}
