/**
 * @file camera_driver.c
 */
#include "camera_driver.h"
#include "xiao_esp32s3_camera_config.h"

#include <stdlib.h>
#include <string.h>
#include "esp_log.h"

static const char *camera_tag = "Camera";

static camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sccb_sda = SIOD_GPIO_NUM,
    .pin_sccb_scl = SIOC_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,

    .xclk_freq_hz = 20000000,          // 外部クロック周波数を20MHzに設定
    .fb_location = CAMERA_FB_IN_PSRAM, // フレームバッファをPSRAMに配置
    .pixel_format = PIXFORMAT_JPEG,    // JPEG圧縮で取得
    .frame_size = FRAMESIZE_VGA,       // 640x480
    .jpeg_quality = 12,                // JPEG品質 (0-63, 小さいほど高品質)
    .fb_count = 2,                     // 使用するフレームバッファの数
    .grab_mode = CAMERA_GRAB_LATEST    // 最新のフレームを取得する
};

esp_err_t camera_init(void) {
    esp_err_t ret = esp_camera_init(&camera_config);
    if (ret != ESP_OK) {
        ESP_LOGE(camera_tag, "Camera init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(camera_tag, "Camera initialized");
    return ESP_OK;
}

esp_err_t camera_get_image(uint8_t *image_buf, size_t buf_size, size_t *image_size) {
    camera_fb_t *pic = esp_camera_fb_get();
    if (!pic) {
        ESP_LOGE(camera_tag, "Failed take picture");
        return ESP_FAIL;
    }

    if (buf_size < pic->len) {
        ESP_LOGE(camera_tag, "Buffer too small. need %d bytes", pic->len);
        esp_camera_fb_return(pic);
        return ESP_ERR_NO_MEM;
    }

    memcpy(image_buf, pic->buf, pic->len);
    *image_size = pic->len;

    esp_camera_fb_return(pic);

    return ESP_OK;
}