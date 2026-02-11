/**
 * @file sdcard_driver.h
 * @brief SPI接続のSDカードドライバ
 *
 * SPI経由でSDカードをFATファイルシステムとしてマウントし、
 * ファイルの読み書きを行うシンプルなドライバ。
 * マウントポイントは "/sdcard" 固定。
 */
#ifndef SDCARD_DRIVER_H
#define SDCARD_DRIVER_H

#include "esp_err.h"
#include "driver/gpio.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief SPI接続のピン設定
 */
typedef struct {
    gpio_num_t mosi;  ///< Master Out Slave In
    gpio_num_t miso;  ///< Master In Slave Out
    gpio_num_t clk;   ///< クロック
    gpio_num_t cs;    ///< チップセレクト
} sdcard_spi_config_t;

/**
 * @brief SDカードをSPIモードで初期化しマウントする
 *
 * SPIバスの初期化、SDカードのマウント、FATファイルシステムの登録を行う。
 * マウントポイントは "/sdcard"。
 *
 * @param config SPIピン設定
 * @return esp_err_t ESP_OK: 成功, その他: エラー
 */
esp_err_t sdcard_init_spi(const sdcard_spi_config_t *config);

/**
 * @brief ファイルにバイナリデータを書き込む（上書き）
 *
 * @param path ファイルのフルパス (例: "/sdcard/image.bin")
 * @param data 書き込むデータ
 * @param len  データサイズ（バイト）
 * @return esp_err_t ESP_OK: 成功, ESP_FAIL: 失敗
 */
esp_err_t sdcard_write(const char *path, const void *data, size_t len);

/**
 * @brief ファイルからバイナリデータを読み込む
 *
 * @param path     ファイルのフルパス (例: "/sdcard/image.bin")
 * @param buf      読み込み先バッファ
 * @param buf_len  バッファサイズ（バイト）
 * @param read_len 実際に読み込んだサイズの格納先
 * @return esp_err_t ESP_OK: 成功, ESP_FAIL: 失敗
 */
esp_err_t sdcard_read(const char *path, void *buf, size_t buf_len, size_t *read_len);

#ifdef __cplusplus
}
#endif

#endif /* SDCARD_DRIVER_H */