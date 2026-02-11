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


static void make_filename(char *buf, size_t buf_len, int seq)
{
    snprintf(buf, buf_len, "/sdcard/img_%04d%s", seq, camera_get_file_ext());
}

static int find_next_seq(void)
{
    int max_seq = -1;
    DIR *dir = opendir("/sdcard");
    if (!dir) {
        return 0;
    }

    const char *ext = camera_get_file_ext();
    char fmt[32];
    snprintf(fmt, sizeof(fmt), "img_%%d%s", ext);

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        int n;
        if (sscanf(ent->d_name, fmt, &n) == 1 && n > max_seq) {
            max_seq = n;
        }
    }
    closedir(dir);

    return max_seq + 1;
}

static void swap_bytes_16(uint8_t *data, size_t size)
{
    for (size_t i = 0; i + 1 < size; i += 2) {
        uint8_t tmp = data[i];
        data[i] = data[i + 1];
        data[i + 1] = tmp;
    }
}

static esp_err_t save_image(const char *path, uint8_t *data, size_t data_size)
{
    char header[128];
    size_t header_size = camera_get_file_header(header, sizeof(header));

    if (header_size == 0) {
        return sdcard_write(path, data, data_size);
    }

    // RGB565: カメラはビッグエンディアンで出力、BMPはリトルエンディアンを期待
    if (camera_get_file_ext()[1] == 'b') { // ".bmp"
        swap_bytes_16(data, data_size);
    }

    FILE *f = fopen(path, "wb");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open file: %s", path);
        return ESP_FAIL;
    }
    size_t written = fwrite(header, 1, header_size, f);
    written += fwrite(data, 1, data_size, f);
    fclose(f);

    if (written != header_size + data_size) {
        ESP_LOGE(TAG, "Write incomplete: %s", path);
        return ESP_FAIL;
    }
    return ESP_OK;
}

static void capture_loop(void)
{
    size_t buf_size = camera_get_buf_size();
    ESP_LOGI(TAG, "Image buffer size: %u bytes", buf_size);
    uint8_t *img_buf = malloc(buf_size);
    if (!img_buf) {
        ESP_LOGE(TAG, "Failed to allocate image buffer");
        return;
    }

    int seq = find_next_seq();
    ESP_LOGI(TAG, "Starting from seq %d", seq);
    while (1) {
        size_t img_size = 0;
        esp_err_t ret = camera_get_image(img_buf, buf_size, &img_size);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to capture image");
            vTaskDelay(pdMS_TO_TICKS(CONFIG_CAPTURE_INTERVAL_MS));
            continue;
        }

        char path[48];
        make_filename(path, sizeof(path), seq);

        ret = save_image(path, img_buf, img_size);
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
