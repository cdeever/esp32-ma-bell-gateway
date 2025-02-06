idf_component_register(SRCS "app_hf_msg_set.c"
                            "bt_app_core.c"
                            "bt_app_hf.c"
                            "gpio_pcm_config.c"
                            "wifi.c"
                            "web.c"
                            "tones.c"
                            "main.c"
                            INCLUDE_DIRS ".")

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error=format)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error=format")