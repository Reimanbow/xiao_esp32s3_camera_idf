/**
 * @file camera_driver.c
 */
#include "camera_driver.h"
#include "xiao_esp32s3_camera_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "sdkconfig.h"

static const char *camera_tag = "Camera";

#if defined(CONFIG_CAMERA_PIXEL_FORMAT_JPEG)
#define CAMERA_PIXEL_FORMAT PIXFORMAT_JPEG
#elif defined(CONFIG_CAMERA_PIXEL_FORMAT_RGB565)
#define CAMERA_PIXEL_FORMAT PIXFORMAT_RGB565
#elif defined(CONFIG_CAMERA_PIXEL_FORMAT_GRAYSCALE)
#define CAMERA_PIXEL_FORMAT PIXFORMAT_GRAYSCALE
#endif

#if defined(CONFIG_CAMERA_FRAME_SIZE_96X96)
#define CAMERA_FRAME_SIZE FRAMESIZE_96X96
#elif defined(CONFIG_CAMERA_FRAME_SIZE_QVGA)
#define CAMERA_FRAME_SIZE FRAMESIZE_QVGA
#elif defined(CONFIG_CAMERA_FRAME_SIZE_VGA)
#define CAMERA_FRAME_SIZE FRAMESIZE_VGA
#elif defined(CONFIG_CAMERA_FRAME_SIZE_SVGA)
#define CAMERA_FRAME_SIZE FRAMESIZE_SVGA
#elif defined(CONFIG_CAMERA_FRAME_SIZE_XGA)
#define CAMERA_FRAME_SIZE FRAMESIZE_XGA
#elif defined(CONFIG_CAMERA_FRAME_SIZE_SXGA)
#define CAMERA_FRAME_SIZE FRAMESIZE_SXGA
#elif defined(CONFIG_CAMERA_FRAME_SIZE_UXGA)
#define CAMERA_FRAME_SIZE FRAMESIZE_UXGA
#endif

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

    .xclk_freq_hz = 20000000,
    .fb_location = CAMERA_FB_IN_PSRAM,
    .pixel_format = CAMERA_PIXEL_FORMAT,
    .frame_size = CAMERA_FRAME_SIZE,
#ifdef CONFIG_CAMERA_JPEG_QUALITY
    .jpeg_quality = CONFIG_CAMERA_JPEG_QUALITY,
#else
    .jpeg_quality = 12,
#endif
    .fb_count = 2,
    .grab_mode = CAMERA_GRAB_LATEST,
};

static const int frame_width[] = {
    [FRAMESIZE_96X96] = 96,  [FRAMESIZE_QVGA] = 320,  [FRAMESIZE_VGA] = 640,
    [FRAMESIZE_SVGA] = 800,  [FRAMESIZE_XGA] = 1024,  [FRAMESIZE_SXGA] = 1280,
    [FRAMESIZE_UXGA] = 1600,
};
static const int frame_height[] = {
    [FRAMESIZE_96X96] = 96,  [FRAMESIZE_QVGA] = 240,  [FRAMESIZE_VGA] = 480,
    [FRAMESIZE_SVGA] = 600,  [FRAMESIZE_XGA] = 768,   [FRAMESIZE_SXGA] = 1024,
    [FRAMESIZE_UXGA] = 1200,
};

const char *camera_get_file_ext(void)
{
    switch (CAMERA_PIXEL_FORMAT) {
    case PIXFORMAT_JPEG:
        return ".jpg";
    case PIXFORMAT_RGB565:
        return ".bmp";
    case PIXFORMAT_GRAYSCALE:
        return ".pgm";
    default:
        return ".bin";
    }
}

static size_t make_bmp_header(uint8_t *buf, size_t buf_size, int w, int h)
{
    const size_t HEADER_SIZE = 66; // 14 (file) + 40 (DIB) + 12 (masks)
    if (buf_size < HEADER_SIZE) {
        return 0;
    }
    memset(buf, 0, HEADER_SIZE);

    uint32_t img_size = w * h * 2;
    uint32_t file_size = HEADER_SIZE + img_size;

    // BMP file header (14 bytes)
    buf[0] = 'B'; buf[1] = 'M';
    memcpy(buf + 2, &file_size, 4);
    uint32_t offset = HEADER_SIZE;
    memcpy(buf + 10, &offset, 4);

    // BITMAPINFOHEADER (40 bytes)
    uint32_t dib_size = 40;
    memcpy(buf + 14, &dib_size, 4);
    int32_t bmp_w = w;
    int32_t bmp_h = -h; // 負値 = top-down (カメラ出力順)
    memcpy(buf + 18, &bmp_w, 4);
    memcpy(buf + 22, &bmp_h, 4);
    uint16_t planes = 1;
    memcpy(buf + 26, &planes, 2);
    uint16_t bpp = 16;
    memcpy(buf + 28, &bpp, 2);
    uint32_t compression = 3; // BI_BITFIELDS
    memcpy(buf + 30, &compression, 4);
    memcpy(buf + 34, &img_size, 4);

    // RGB565 color masks
    uint32_t r_mask = 0xF800;
    uint32_t g_mask = 0x07E0;
    uint32_t b_mask = 0x001F;
    memcpy(buf + 54, &r_mask, 4);
    memcpy(buf + 58, &g_mask, 4);
    memcpy(buf + 62, &b_mask, 4);

    return HEADER_SIZE;
}

size_t camera_get_file_header(char *buf, size_t buf_size)
{
    int w = frame_width[CAMERA_FRAME_SIZE];
    int h = frame_height[CAMERA_FRAME_SIZE];

    switch (CAMERA_PIXEL_FORMAT) {
    case PIXFORMAT_GRAYSCALE:
        return snprintf(buf, buf_size, "P5\n%d %d\n255\n", w, h);
    case PIXFORMAT_RGB565:
        return make_bmp_header((uint8_t *)buf, buf_size, w, h);
    default:
        return 0;
    }
}

size_t camera_get_buf_size(void)
{
    size_t pixels = frame_width[CAMERA_FRAME_SIZE] * frame_height[CAMERA_FRAME_SIZE];

    switch (CAMERA_PIXEL_FORMAT) {
    case PIXFORMAT_RGB565:
        return pixels * 2;
    case PIXFORMAT_GRAYSCALE:
        return pixels;
    case PIXFORMAT_JPEG:
    default:
        return pixels / 5; // JPEG は raw の ~20% 程度を目安に確保
    }
}

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