idf_component_register(
    SRCS "app/state/ma_bell_state.c"
            "app/bluetooth/app_hf_msg_set.c"
            "app/tones.c"
            "app/web/web_interface.c"
            "app/events/event_system.c"
            "storage/storage.c"
            "bluetooth/bt_app_core.c"
            "bluetooth/bt_app_hf.c"
            "bluetooth/bt_init.c"
            "bluetooth/bt_connection_manager.c"
            "hardware/gpio_pcm_config.c"
            "hardware/hardware_init.c"
            "network/wifi/wifi.c"
            "network/wifi/wifi_init.c"
            "network/mqtt/mqtt.c"
            "main.c"
            INCLUDE_DIRS "." "app" "app/state" "app/bluetooth" "app/web" "app/events" "storage" "bluetooth" "hardware" "network/wifi" "network/mqtt" "config"
            REQUIRES nvs_flash driver bt esp_wifi esp_common console esp_http_server mqtt
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error=format)