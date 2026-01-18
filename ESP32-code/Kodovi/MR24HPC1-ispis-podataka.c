#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"

#define UART_PORT UART_NUM_2
#define TX_PIN 17
#define RX_PIN 16
#define BUF_SIZE 256

static const char *TAG = "MR24HPC1";

/* ===== STRUKTURA PODATAKA ===== */
typedef struct {
    uint8_t cmd;       // tip framea (0x80)
    uint8_t presence;  // 0 = nema, 1 = ima
    uint8_t value;     // vrijednost
} mr24_data_t;

/* ===== PROTOTIPI ===== */
void uart_init(void);
void mr24_task(void *arg);
bool mr24_parse(uint8_t *buf, int len, mr24_data_t *out);

/* ===== UART INIT ===== */
void uart_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, TX_PIN, RX_PIN,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

/* ===== PARSER FRAME-a ===== */
bool mr24_parse(uint8_t *buf, int len, mr24_data_t *out)
{
    if (len < 10) return false;

    // Header i Footer
    if (buf[0] == 0x53 && buf[1] == 0x59 &&
        buf[8] == 0x54 && buf[9] == 0x43)   
    {
        out->cmd      = buf[2];
        out->presence = buf[5];
        out->value    = buf[6];
        return true;
    }

    return false;
}

/* ===== TASK ZA ČITANJE UART-a ===== */
void mr24_task(void *arg)
{
    uint8_t buf[BUF_SIZE];
    mr24_data_t data;

    while (1) {
        int len = uart_read_bytes(UART_PORT, buf, BUF_SIZE,
                                  pdMS_TO_TICKS(1000));

        if (len <= 0) continue;

        for (int i = 0; i <= len - 10; i++) {
            if (mr24_parse(&buf[i], 10, &data)) {
                ESP_LOGI(TAG, "====== MR24HPC1 ======");
                ESP_LOGI(TAG, "CMD       : 0x%02X", data.cmd);
                ESP_LOGI(TAG, "Presence  : %s", data.presence ? "DA" : "NE");
                ESP_LOGI(TAG, "Vrijednost: %d", data.value);
            }
        }
    }
}

/* ===== MAIN ===== */
void app_main(void)
{
    uart_init();
    xTaskCreate(mr24_task, "mr24_task", 4096, NULL, 5, NULL);
}
