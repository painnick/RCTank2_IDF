/**
 * @file rctank.c
 * @brief RC Tank 통합 초기화 (README 초기화 순서)
 */
#include "rctank.h"
#include "rctank_motor.h"
#include "rctank_servo.h"
#include "rctank_led.h"
#include "rctank_dfplayer.h"
#include "rctank_storage.h"

#include "esp_log.h"

static const char *TAG = "rctank";

esp_err_t rctank_init(void)
{
    esp_err_t ret;

    ret = rctank_storage_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "storage init: %s, use defaults", esp_err_to_name(ret));
    }

    ret = rctank_servo_init();
    if (ret != ESP_OK) return ret;

    ret = rctank_led_init();
    if (ret != ESP_OK) return ret;

    ret = rctank_motor_init();
    if (ret != ESP_OK) return ret;

    ret = rctank_dfplayer_init();
    if (ret != ESP_OK) return ret;

    uint8_t vol = rctank_storage_volume_get();
    rctank_dfplayer_set_volume(vol);
    rctank_dfplayer_play(RCTANK_DFPLAYER_TRACK_IDLE);

    ESP_LOGI(TAG, "rctank init complete (vol=%u)", (unsigned)vol);
    return ESP_OK;
}
