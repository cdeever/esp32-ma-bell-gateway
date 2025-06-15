#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "ma_bell.h"

volatile int32_t system_state = SYSTEM_STATE_IDLE;  

void set_system_state(int32_t new_state) {
    system_state = new_state;
}

int32_t get_system_state(void) {
    return system_state;
}

