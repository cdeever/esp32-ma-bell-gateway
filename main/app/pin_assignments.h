#pragma once

#include "driver/gpio.h"

// Phone Line & Audio Interface
#define PIN_PCM_FSYNC        GPIO_NUM_25  // I2S/PCM word select (frame sync, "LRCLK")
#define PIN_PCM_CLK_OUT      GPIO_NUM_5   // I2S/PCM bit clock ("BCLK")
#define PIN_PCM_DOUT         GPIO_NUM_26  // I2S/PCM audio out to phone line interface
#define PIN_PCM_DIN          GPIO_NUM_35  // I2S/PCM audio in from phone (if bidirectional)
#define PIN_OFF_HOOK_DETECT  GPIO_NUM_32  // Input from off-hook detection circuit
#define PIN_RING_DETECT      GPIO_NUM_33  // Input from ring detection circuit
#define PIN_DIAL_LAMP_CTRL   GPIO_NUM_27  // Output to dial lamp relay/MOSFET
#define PIN_PULSE_DIAL_IN    GPIO_NUM_34  // Input from rotary pulse detection
#define PIN_DTMF_IN          GPIO_NUM_39  // Input from DTMF decoder IC (if used)

// Bluetooth Module
#define PIN_BT_RESET         GPIO_NUM_16  // Output: Reset line for Bluetooth module
#define PIN_BT_POWER_CTRL    GPIO_NUM_17  // Output: Power enable for BT module
#define PIN_UART0_TX         GPIO_NUM_1   // Debug serial output (do not use for app I/O)
#define PIN_UART0_RX         GPIO_NUM_3   // Debug serial input

// User Interface
#define PIN_STATUS_LED       GPIO_NUM_2   // Output: General status indication
#define PIN_USER_BUTTON      GPIO_NUM_4   // Input: User pushbutton (pairing, reset, etc)
#define PIN_UI_LED           GPIO_NUM_18  // Output: Additional status LED
#define PIN_I2C_SDA          GPIO_NUM_21  // Data line for I2C display/expander (if used)
#define PIN_I2C_SCL          GPIO_NUM_22  // Clock line for I2C display/expander

// Reserved/Unavailable Pins
// GPIO6-11: Used internally for flash
// GPIO0, 12, 15: Used for boot strapping
// GPIO34-39: Input-only pins

// Acoustic Echo Cancellation
#define PIN_AEC_1            GPIO_NUM_19  // AEC control line 1
#define PIN_AEC_2            GPIO_NUM_21  // AEC control line 2
#define PIN_AEC_3            GPIO_NUM_22  // AEC control line 3

// Function to validate pin assignments
esp_err_t validate_pin_assignments(void); 