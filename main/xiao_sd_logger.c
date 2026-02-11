#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdcard_driver.h"
#include "camera_driver.h"
#include "log_router.h"

static const char *TAG = "main";

#define IMG_BUF_SIZE (100 * 1024)

static void make_filename(char *buf, size_t buf_len, int seq)
{
    snprintf(buf, buf_len, "/sdcard/img_%04d.jpg", seq);
}

static int find_next_seq(void)
{
    int max_seq = -1;
    DIR *dir = opendir("/sdcard");
    if (!dir) {
        return 0;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        int n;
        if (sscanf(ent->d_name, "img_%d.jpg", &n) == 1 && n > max_seq) {
            max_seq = n;
        }
    }
    closedir(dir);

    return max_seq + 1;
}

static void capture_loop(void)
{
    uint8_t *img_buf = malloc(IMG_BUF_SIZE);
    if (!img_buf) {
        ESP_LOGE(TAG, "Failed to allocate image buffer");
        return;
    }

    int seq = find_next_seq();
    ESP_LOGI(TAG, "Starting from seq %d", seq);
    while (1) {
        size_t img_size = 0;
        esp_err_t ret = camera_get_image(img_buf, IMG_BUF_SIZE, &img_size);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to capture image");
            vTaskDelay(pdMS_TO_TICKS(CONFIG_CAPTURE_INTERVAL_MS));
            continue;
        }

        char path[32];
        make_filename(path, sizeof(path), seq);

        ret = sdcard_write(path, img_buf, img_size);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "[%04d] Saved %u bytes -> %s", seq, img_size, path);
            seq++;
        } else {
            ESP_LOGE(TAG, "Failed to write %s", path);
        }

        vTaskDelay(pdMS_TO_TICKS(CONFIG_CAPTURE_INTERVAL_MS));
    }
}

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

    ret = esp_log_router_to_file("/sdcard/log.txt", NULL, ESP_LOG_INFO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Log router init failed: %s", esp_err_to_name(ret));
    }
    ESP_LOGI(TAG, "=== BOOT ===");

    ret = camera_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed: %s", esp_err_to_name(ret));
        return;
    }

    capture_loop();
}
