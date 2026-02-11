/**
 * @file camera_driver.h
 * @brief カメラドライバのインターフェース
 *
 * XIAO ESP32S3 Sense 搭載の OV2640 カメラを制御し、
 * JPEG 画像の取得機能を提供する。
 */
#ifndef CAMERA_DRIVER_H
#define CAMERA_DRIVER_H

#include "esp_camera.h"
#include "esp_err.h"

/**
 * @brief カメラを初期化する
 * @return ESP_OK: 成功, その他: エラー
 */
esp_err_t camera_init(void);

/**
 * @brief カメラから JPEG 画像を取得する
 * @param[out] image_buf  画像データの格納先バッファ
 * @param[in]  buf_size   バッファサイズ (bytes)
 * @param[out] image_size 実際に取得した画像サイズ (bytes)
 * @return ESP_OK: 成功, ESP_ERR_INVALID_SIZE: バッファ不足, その他: エラー
 */
esp_err_t camera_get_image(uint8_t *image_buf, size_t buf_size, size_t *image_size);

#endif /* CAMERA_DRIVER_H */