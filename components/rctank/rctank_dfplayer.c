/**
 * @file rctank_dfplayer.c
 * @brief DFPlayer Mini UART 프로토콜 (ESP-IDF driver/uart)
 */
#include "rctank_dfplayer.h"
#include "rctank_pins.h"

#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "rctank_dfplayer";

#define UART_NUM            UART_NUM_1
#define UART_BAUD            9600
#define UART_BUF_SIZE        256
#define DFPLAYER_CMD_LEN     10

static uint16_t dfplayer_checksum(uint8_t *buf, int len)
{
    uint16_t sum = 0;
    for (int i = 1; i < len - 1; i++) {
        sum += buf[i];
    }
    return (uint16_t)(-sum);
}

static esp_err_t dfplayer_send_cmd(uint8_t cmd, uint8_t param_h, uint8_t param_l)
{
    uint8_t buf[DFPLAYER_CMD_LEN] = {
        0x7E, 0xFF, 0x06, cmd, 0x00, param_h, param_l, 0x00, 0x00, 0xEF
    };
    uint16_t cs = dfplayer_checksum(buf, DFPLAYER_CMD_LEN);
    buf[7] = (uint8_t)(cs >> 8);
    buf[8] = (uint8_t)(cs & 0xFF);
    int n = uart_write_bytes(UART_NUM, buf, DFPLAYER_CMD_LEN);
    if (n != DFPLAYER_CMD_LEN) {
        ESP_LOGE(TAG, "uart_write_bytes %d", n);
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t rctank_dfplayer_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    esp_err_t ret = uart_param_config(UART_NUM, &uart_config);
    if (ret != ESP_OK) return ret;
    ret = uart_set_pin(UART_NUM, RCTANK_PIN_DFPLAYER_TX, RCTANK_PIN_DFPLAYER_RX, -1, -1);
    if (ret != ESP_OK) return ret;
    ret = uart_driver_install(UART_NUM, UART_BUF_SIZE, UART_BUF_SIZE, 0, NULL, 0);
    if (ret != ESP_OK) return ret;

    vTaskDelay(pdMS_TO_TICKS(200));
    ESP_LOGI(TAG, "dfplayer init ok");
    return ESP_OK;
}

esp_err_t rctank_dfplayer_play(uint8_t track)
{
    if (track < 1 || track > 99) return ESP_ERR_INVALID_ARG;
    return dfplayer_send_cmd(0x0C, 0x00, track);
}

esp_err_t rctank_dfplayer_set_volume(uint8_t vol)
{
    if (vol > 30) vol = 30;
    return dfplayer_send_cmd(0x06, 0x00, vol);
}

esp_err_t rctank_dfplayer_stop(void)
{
    return dfplayer_send_cmd(0x16, 0x00, 0x00);
}
