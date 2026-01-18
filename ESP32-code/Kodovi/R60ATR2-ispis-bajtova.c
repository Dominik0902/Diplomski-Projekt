#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"

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

void R60_task(void *arg) {
    uint8_t data[BUF_SIZE];

    while (1) {
        int len = uart_read_bytes(UART_PORT, data, BUF_SIZE, pdMS_TO_TICKS(1000));
        if (len > 0) {
            ESP_LOGI(TAG, "Primljeno %d bajtova:", len);
            for (int i = 0; i < len; i++){
                printf("%02X ", data[i]);
            }
        }
    }
}

void app_main(void) {
    R60_uart_init();
    xTaskCreate(R60_task, "R60_read_task", 4096, NULL, 10, NULL);
}
