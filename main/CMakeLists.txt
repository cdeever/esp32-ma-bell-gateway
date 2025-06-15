idf_component_register(
    SRCS "app/state/ma_bell_state.c"
            "app/bluetooth/app_hf_msg_set.c"
            "app/tones.c"
            "network/wifi.c"
            "network/web/web_interface.c"
            "storage/storage.c"
            "bluetooth/bt_app_core.c"
            "bluetooth/bt_app_hf.c"
            "hardware/gpio_pcm_config.c"
            "main.c"
            INCLUDE_DIRS "." "app" "app/state" "app/bluetooth" "network" "network/web" "storage" "bluetooth" "hardware"
            REQUIRES nvs_flash driver bt esp_wifi esp_common console esp_http_server
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error=format)