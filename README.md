# xiao_sd_logger

XIAO ESP32S3 Sense のカメラで定期的に JPEG 画像を撮影し、SD カードに連番で保存するロガー。

## 動作

1. SD カード (SPI接続) とカメラ (OV3660) を初期化
2. SD カード上の既存ファイルをスキャンし、連番の続きから保存を開始
3. 一定間隔 (デフォルト 5秒) で JPEG 画像を撮影し、`img_0000.jpg`, `img_0001.jpg`, ... と連番で SD カードに保存

## ハードウェア

- [XIAO ESP32S3 Sense](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)
- microSD カード (FAT フォーマット)

### ピン接続

| 機能 | GPIO |
|------|------|
| SD MOSI | 9 |
| SD MISO | 8 |
| SD CLK | 7 |
| SD CS | 21 |
| カメラ | ボード内蔵 (XIAO ESP32S3 Sense) |

## 必要なもの

- Docker
- [just](https://github.com/casey/just)
- [esptool](https://github.com/espressif/esptool) (`pip install esptool`) - 書き込み用
- [pyserial](https://pypi.org/project/pyserial/) (`pip install pyserial`) - モニター用

## セットアップ

```bash
git clone <リポジトリURL>
cd xiao_sd_logger

# 初回のみ: ターゲット設定 + ビルド
just init chip=esp32s3
just build
```

## ビルド・書き込み

```bash
# ビルド
just build

# 書き込み
just flash

# シリアルモニター
just monitor

# ビルド → 書き込み → モニター 一括
just run
```

## 設定の変更

撮影間隔などの設定は `just menuconfig` で変更できる。設定値は `sdkconfig` に保存され、git 管理外のためリポジトリを汚さない。

```bash
just menuconfig
```

| 設定項目 | 場所 | デフォルト |
|----------|------|-----------|
| 撮影間隔 | App Configuration > Capture interval (ms) | 5000 |
| PSRAM | Component config > ESP PSRAM | 有効 (Octal Mode) |
| FAT LFN | Component config > FAT Filesystem support > Long filename support | Heap |

初回ビルド時に `sdkconfig` が存在しない場合、`sdkconfig.defaults` の値が適用される。

## コマンド一覧

| コマンド | 説明 |
|---|---|
| `just init chip=esp32s3` | プロジェクト初期化 + ターゲット設定 |
| `just build` | ビルド |
| `just menuconfig` | SDK 設定画面 (TUI) |
| `just flash` | 書き込み |
| `just monitor` | シリアルモニター |
| `just run` | build → flash → monitor 一括実行 |
| `just clean` | ビルド成果物を削除 |
| `just fullclean` | ビルドディレクトリ全体を削除 |
| `just size` | メモリ使用量の確認 |

### オプション引数

```bash
# ポートを指定 (デフォルト: /dev/ttyACM0)
just flash port=/dev/ttyUSB0

# ボーレートを指定
just flash baud=921600

# モニターのボーレート (デフォルト: 115200)
just monitor baud=9600
```

## VS Code で補完を有効にする

VS Code の **Dev Containers** 拡張をインストールし、コマンドパレットから `Dev Containers: Reopen in Container` を実行する。ESP-IDF のヘッダーが解決され、補完・定義ジャンプが有効になる。

ビルド・書き込み・モニターはホスト側のターミナルで `just` を使う。

## プロジェクト構成

```
.
├── Justfile                    # ビルド・書き込み等のコマンド定義
├── CMakeLists.txt              # プロジェクトの CMake 設定
├── sdkconfig.defaults          # デフォルトの SDK 設定
├── .clang-format               # コードフォーマット設定
├── main/
│   ├── CMakeLists.txt
│   ├── Kconfig.projbuild       # menuconfig のアプリ設定定義
│   └── xiao_sd_logger.c        # エントリポイント (app_main)
└── components/
    ├── sdcard_driver/           # SD カード (SPI) ドライバ
    │   ├── include/
    │   │   └── sdcard_driver.h
    │   └── sdcard_driver.c
    └── camera_driver/           # カメラドライバ
        ├── include/
        │   ├── camera_driver.h
        │   └── xiao_esp32s3_camera_config.h
        └── camera_driver.c
```
