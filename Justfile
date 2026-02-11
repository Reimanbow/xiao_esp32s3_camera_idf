# 変数定義
PROJECT_ROOT := justfile_directory()
# ESP-IDF バージョン (変更はここだけ / IDF_VERSION=v5.3 just build で一時的に上書き可)
IDF_VERSION := env_var_or_default("IDF_VERSION", "v5.4")
IMAGE := "espressif/idf:" + IDF_VERSION
UID := `id -u`
GID := `id -g`
DOCKER_RUN := "docker run --rm -u " + UID + ":" + GID + " -v " + PROJECT_ROOT + ":/workspace -w /workspace"
DOCKER_RUN_IT := "docker run --rm -it -u " + UID + ":" + GID + " -v " + PROJECT_ROOT + ":/workspace -w /workspace"
PROJECT_NAME := file_name(PROJECT_ROOT)
CHIP := `grep '^CONFIG_IDF_TARGET=' sdkconfig | sed 's/CONFIG_IDF_TARGET="\(.*\)"/\1/' || echo "esp32"`

# プロジェクトの初期化(生成)
init chip="esp32":
    # コンテナを立ち上げ、プロジェクトファイルを生成
    {{DOCKER_RUN}} {{IMAGE}} idf.py create-project {{PROJECT_NAME}}
    # 生成されたファイルをルートに移動し、不要なディレクトリを整理
    mv {{PROJECT_NAME}}/* .
    rm -rf {{PROJECT_NAME}}
    # ターゲットチップを設定
    {{DOCKER_RUN}} {{IMAGE}} idf.py set-target {{chip}}
    @echo "プロジェクト '{{PROJECT_NAME}}' ({{chip}}) が立ち上がりました"
  
# ビルド
build:
    {{DOCKER_RUN}} {{IMAGE}} idf.py build

# 設定(チップ固有の設定画面を開く)
menuconfig:
    {{DOCKER_RUN_IT}} {{IMAGE}} idf.py menuconfig

# 書き込み
flash port="/dev/ttyACM0" baud="460800":
    @echo "検知されたチップ: {{CHIP}}"
    cd build && esptool.py --chip {{CHIP}} -p {{port}} -b {{baud}} --before default_reset --after hard_reset write_flash @flash_project_args

monitor port="/dev/ttyACM0" baud="115200":
    pyserial-miniterm {{port}} {{baud}}

# ビルド→書き込み→モニター一括実行
run: build flash monitor

# ライブラリ追加 (例: just add-lib espressif/button)
add-lib name:
    {{DOCKER_RUN}} {{IMAGE}} idf.py add-dependency "{{name}}"

# ローカルコンポーネント作成 (例: just create-comp my_sensor)
create-comp name:
    mkdir -p components
    {{DOCKER_RUN}} {{IMAGE}} idf.py -C components create-component "{{name}}"

# ビルド成果物を削除
clean:
    {{DOCKER_RUN}} {{IMAGE}} idf.py clean

# 完全削除（CMakeキャッシュ含む）
fullclean:
    {{DOCKER_RUN}} {{IMAGE}} idf.py fullclean

# メモリ使用量の確認
size:
    {{DOCKER_RUN}} {{IMAGE}} idf.py size