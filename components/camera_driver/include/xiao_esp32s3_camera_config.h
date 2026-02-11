/**
 * @file xiao_esp32s3_camera_config.h
 * @brief XIAO ESP32S3 Sense ボードのカメラ (OV2640) ピン定義
 *
 * @see https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/
 */
#ifndef XIAO_ESP32S3_CAMERA_CONFIG_H
#define XIAO_ESP32S3_CAMERA_CONFIG_H

/* 電源・リセット (未使用: -1) */
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1

/* XCLK: カメラへのクロック供給 */
#define XCLK_GPIO_NUM     10

/* SCCB (I2C): カメラレジスタ制御 */
#define SIOD_GPIO_NUM     40
#define SIOC_GPIO_NUM     39

/* D0-D7: 8bit パラレルデータバス */
#define Y9_GPIO_NUM       48
#define Y8_GPIO_NUM       11
#define Y7_GPIO_NUM       12
#define Y6_GPIO_NUM       14
#define Y5_GPIO_NUM       16
#define Y4_GPIO_NUM       18
#define Y3_GPIO_NUM       17
#define Y2_GPIO_NUM       15

/* 同期信号 */
#define VSYNC_GPIO_NUM    38
#define HREF_GPIO_NUM     47
#define PCLK_GPIO_NUM     13

/* オンボード LED */
#define LED_GPIO_NUM      21

#endif /* XIAO_ESP32S3_CAMERA_CONFIG_H */