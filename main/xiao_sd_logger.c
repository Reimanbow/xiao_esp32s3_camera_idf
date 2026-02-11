#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdcard_driver.h"
#include "camera_driver.h"

static const char *filename = "/sdcard/img.jpg";

static const char *TAG = "main";
static const char *TEST_FILE = "/sdcard/test.txt";
static const char *TEST_DATA = "Hello, SD card!";

void app_main(void)
{
    sdcard_spi_config_t cfg = {
        .mosi = GPIO_NUM_9,
        .miso = GPIO_NUM_8,
        .clk  = GPIO_NUM_7,
        .cs   = GPIO_NUM_21,
    };

    esp_err_t ret = sdcard_init_spi(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SD init failed: %s", esp_err_to_name(ret));
        return;
    }

    ret = sdcard_write(TEST_FILE, TEST_DATA, strlen(TEST_DATA));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Write failed");
        return;
    }
    ESP_LOGI(TAG, "Wrote: %s", TEST_DATA);

    ret = camera_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed: %s", esp_err_to_name(ret));
        return;
    }

    size_t buf_size = 100 * 1024; // 100KB (VGA JPEG は通常 20〜80KB)
    uint8_t *img_buf = malloc(buf_size);
    if (!img_buf) {
        ESP_LOGE(TAG, "Failed allocate memory");
        return;
    }

    size_t img_size = 0;
    ret = camera_get_image(img_buf, buf_size, &img_size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get image");
        free(img_buf);
        return;
    }

    ESP_LOGI(TAG, "Captured image: %u bytes", img_size);
    sdcard_write(filename, img_buf, img_size);
    free(img_buf);

    char buf[64];
    while (1) {
        size_t read_len = 0;
        ret = sdcard_read(TEST_FILE, buf, sizeof(buf) - 1, &read_len);
        if (ret == ESP_OK) {
            buf[read_len] = '\0';
            ESP_LOGI(TAG, "Read: %s", buf);
        } else {
            ESP_LOGE(TAG, "Read failed");
        }
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
