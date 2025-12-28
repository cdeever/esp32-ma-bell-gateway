#include "pin_assignments.h"
#include "esp_log.h"

static const char *TAG = "pin_assignments";

esp_err_t validate_pin_assignments(void) {
    // Check for conflicts with reserved pins
    const gpio_num_t reserved_pins[] = {
        GPIO_NUM_6, GPIO_NUM_7, GPIO_NUM_8, GPIO_NUM_9, GPIO_NUM_10, GPIO_NUM_11,  // Flash
        GPIO_NUM_0, GPIO_NUM_12, GPIO_NUM_15  // Boot strapping
    };
    
    const gpio_num_t input_only_pins[] = {
        GPIO_NUM_34, GPIO_NUM_35, GPIO_NUM_36, GPIO_NUM_37, GPIO_NUM_38, GPIO_NUM_39
    };

    // Check if any of our pins are in the reserved list
    const gpio_num_t our_pins[] = {
        PIN_PCM_FSYNC, PIN_PCM_CLK_OUT, PIN_PCM_DOUT, PIN_PCM_DIN,
        PIN_OFF_HOOK_DETECT, PIN_RING_DETECT, PIN_DIAL_LAMP_CTRL,
        PIN_PULSE_DIAL_IN, PIN_DTMF_IN, PIN_BT_RESET, PIN_BT_POWER_CTRL,
        PIN_UART0_TX, PIN_UART0_RX, PIN_STATUS_LED, PIN_USER_BUTTON,
        PIN_UI_LED, PIN_I2C_SDA, PIN_I2C_SCL
    };

    // Check for conflicts with reserved pins
    for (size_t i = 0; i < sizeof(our_pins)/sizeof(our_pins[0]); i++) {
        for (size_t j = 0; j < sizeof(reserved_pins)/sizeof(reserved_pins[0]); j++) {
            if (our_pins[i] == reserved_pins[j]) {
                ESP_LOGE(TAG, "Pin conflict: GPIO%d is reserved", our_pins[i]);
                return ESP_ERR_INVALID_STATE;
            }
        }
    }

    // Check if output pins are assigned to input-only pins
    const gpio_num_t output_pins[] = {
        PIN_PCM_CLK_OUT, PIN_PCM_DOUT, PIN_DIAL_LAMP_CTRL,
        PIN_BT_RESET, PIN_BT_POWER_CTRL, PIN_UART0_TX,
        PIN_STATUS_LED, PIN_UI_LED, PIN_I2C_SDA, PIN_I2C_SCL
    };

    for (size_t i = 0; i < sizeof(output_pins)/sizeof(output_pins[0]); i++) {
        for (size_t j = 0; j < sizeof(input_only_pins)/sizeof(input_only_pins[0]); j++) {
            if (output_pins[i] == input_only_pins[j]) {
                ESP_LOGE(TAG, "Invalid assignment: GPIO%d is input-only but used as output", output_pins[i]);
                return ESP_ERR_INVALID_STATE;
            }
        }
    }

    ESP_LOGI(TAG, "Pin assignments validated successfully");
    return ESP_OK;
} 