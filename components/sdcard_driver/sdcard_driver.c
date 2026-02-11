/**
 * @file sdcard_driver.c
 */
#include "sdcard_driver.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "esp_log.h"

// SDカードのハンドルを保持するためのポインタ
static sdmmc_card_t *card;

static const char *sdcard_tag = "SD";
static const char *mount_dir = "/sdcard";

esp_err_t sdcard_init_spi(const sdcard_spi_config_t *config) {
    esp_err_t ret;
    ESP_LOGI(sdcard_tag, "Initializing SD card");

    // SPIホスト設定を取得する
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = config->mosi,
        .miso_io_num = config->miso,
        .sclk_io_num = config->clk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };

    // 定義されたピンでSPIパスを初期化する
    ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(sdcard_tag, "Initializing SD card");
        return ret;
    }

    // SDSPIデバイスのデフォルト設定を取得する. CSピンを設定する
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = config->cs;
    slot_config.host_id = host.slot;

    // VFS-FATマウント設定
    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    // SPIバスを通じてSDカードをマウントし、FATファイルシステムを/sdに登録する
    ret = esp_vfs_fat_sdspi_mount(mount_dir, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        ESP_LOGE(sdcard_tag, "Failed to mount filesystem");
        return ret;
    }

    ESP_LOGI(sdcard_tag, "Finished Initilizing SD card");
    return ret;
}

esp_err_t sdcard_write(const char *path, const void *data, size_t len) {
	FILE *f = fopen(path, "wb");
	if (!f) {
		ESP_LOGE(sdcard_tag, "Failed to open file for writing: %s", path);
		return ESP_FAIL;
	}
	size_t written = fwrite(data, 1, len, f);
	fclose(f);

	if (written != len) {
		ESP_LOGE(sdcard_tag, "Write incomplete: %s", path);
		return ESP_FAIL;
	}
	return ESP_OK;
}

esp_err_t sdcard_read(const char *path, void *buf, size_t buf_len, size_t *read_len) {
	FILE *f = fopen(path, "rb");
	if (!f) {
		ESP_LOGE(sdcard_tag, "Failed to open file for reading: %s", path);
		return ESP_FAIL;
	}
	*read_len = fread(buf, 1, buf_len, f);
	fclose(f);
	return ESP_OK;
}