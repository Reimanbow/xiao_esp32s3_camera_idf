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

/**
 * @brief 現在のピクセルフォーマットに対応するファイル拡張子を取得する
 * @return 拡張子文字列 (例: ".jpg", ".pgm", ".rgb565")
 */
const char *camera_get_file_ext(void);

/**
 * @brief 現在のフォーマットに必要なファイルヘッダを生成する
 *
 * PGM等、ヘッダが必要なフォーマットの場合にヘッダを buf に書き込む。
 * JPEG等ヘッダ不要なフォーマットでは 0 を返す。
 *
 * @param buf ヘッダ書き込み先バッファ
 * @param buf_size バッファサイズ
 * @return 書き込まれたヘッダのバイト数 (0 = ヘッダ不要)
 */
size_t camera_get_file_header(char *buf, size_t buf_size);

/**
 * @brief 画像1枚分の推奨バッファサイズを取得する
 * @return バッファサイズ (bytes)
 */
size_t camera_get_buf_size(void);

#endif /* CAMERA_DRIVER_H */