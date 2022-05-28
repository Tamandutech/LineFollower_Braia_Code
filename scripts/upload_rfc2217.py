from os.path import join

Import("env")

platform = env.PioPlatform()
board = env.BoardConfig()
mcu = board.get("build.mcu", "esp32")

env.Replace(
    UPLOADER=join(platform.get_package_dir("tool-esptoolpy") or "", "esptool.py"),
    UPLOADERFLAGS=[
        "--chip", mcu,
        "--port", '"$UPLOAD_PORT"',
        "--baud", "$UPLOAD_SPEED",
        "--before", "default_reset",
        "--after", "hard_reset",
        "write_flash", "-z",
        "--flash_mode", "${__get_board_flash_mode(__env__)}",
        "--flash_freq", "${__get_board_f_flash(__env__)}",
        "--flash_size", "detect"
    ],
    UPLOADCMD='"$PYTHONEXE" "$UPLOADER" $UPLOADERFLAGS $ESP32_APP_OFFSET $SOURCE'
)

for image in env.get("FLASH_EXTRA_IMAGES", []):
    env.Append(UPLOADERFLAGS=[image[0], env.subst(image[1])])

if "uploadfs" in COMMAND_LINE_TARGETS:
    env.Replace(
        UPLOADERFLAGS=[
            "--chip", mcu,
            "--port", '"$UPLOAD_PORT"',
            "--baud", "$UPLOAD_SPEED",
            "--before", "default_reset",
            "--after", "hard_reset",
            "write_flash", "-z",
            "--flash_mode", "$BOARD_FLASH_MODE",
            "--flash_size", "detect",
            "$SPIFFS_START"
        ],
        UPLOADCMD='"$PYTHONEXE" "$UPLOADER" $UPLOADERFLAGS $SOURCE',
    )

