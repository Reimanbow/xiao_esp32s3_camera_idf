# idf-template

ESP-IDF の開発環境テンプレート。Docker + [just](https://github.com/casey/just) でホストへの ESP-IDF インストール不要。

```bash
gh repo clone <ユーザー名>/idf-template my-project
# または
git clone <リポジトリURL> my-project

cd my-project

# git 履歴をリセットして自分のリポジトリにする
rm -rf .git && git init
# または remote の向き先だけ変更する場合
git remote set-url origin <自分のリポジトリURL>

just init chip=esp32s3
```

ディレクトリ名がそのままプロジェクト名になります。

## 必要なもの

- Docker
- [just](https://github.com/casey/just)
- [esptool](https://github.com/espressif/esptool) (`pip install esptool`) - 書き込み用
- [pyserial](https://pypi.org/project/pyserial/) (`pip install pyserial`) - モニター用

## クイックスタート

```bash
# プロジェクト初期化 (デフォルト: esp32)
just init

# ESP32-S3 の場合
just init chip=esp32s3

# ビルド
just build

# 書き込み
just flash

# シリアルモニター
just monitor

# ビルド → 書き込み → モニター 一括
just run
```

## コマンド一覧

| コマンド | 説明 |
|---|---|
| `just init chip=esp32` | プロジェクト初期化 + ターゲット設定 |
| `just build` | ビルド |
| `just menuconfig` | SDK 設定画面 (TUI) |
| `just flash` | 書き込み |
| `just monitor` | シリアルモニター |
| `just run` | build → flash → monitor 一括実行 |
| `just add-lib <name>` | ESP Component Registry からライブラリ追加 |
| `just create-comp <name>` | ローカルコンポーネント作成 |
| `just clean` | ビルド成果物を削除 |
| `just fullclean` | ビルドディレクトリ全体を削除 |
| `just size` | メモリ使用量の確認 |

## オプション引数

```bash
# ポートを指定 (デフォルト: /dev/ttyACM0)
just flash port=/dev/ttyUSB0

# ボーレートを指定
just flash baud=921600

# モニターのボーレート (デフォルト: 115200)
just monitor baud=9600
```

## ESP-IDF バージョンの変更

```bash
# 一時的に変更
IDF_VERSION=v5.3 just build

# プロジェクト固定 (.env ファイル)
echo 'IDF_VERSION=v5.3' > .env
```

## VS Code で補完を有効にする

VS Code の **Dev Containers** 拡張をインストールし、コマンドパレットから `Dev Containers: Reopen in Container` を実行するとコンテナ内で VS Code が開きます。ESP-IDF のヘッダーが全て解決され、補完・定義ジャンプ・エラー表示が有効になります。

ビルド・書き込み・モニターはホスト側のターミナルで `just` を使ってください。

## プロジェクト構成

```
.
├── Justfile              # ビルド・書き込み等のコマンド定義
├── CMakeLists.txt        # プロジェクトの CMake 設定
├── sdkconfig.defaults    # デフォルトの SDK 設定
├── .clang-format         # コードフォーマット設定
├── main/
│   ├── CMakeLists.txt
│   └── <project_name>.c  # エントリポイント (app_main)
└── components/           # ローカルコンポーネント (任意)
```
