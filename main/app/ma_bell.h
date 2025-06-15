#ifndef MA_BELL_H
#define MA_BELL_H

#include <stdint.h>

// Define system states (bitmasks)
#define SYSTEM_STATE_IDLE         0x01
#define SYSTEM_STATE_PAIRING      0x02
#define SYSTEM_STATE_ACTIVE_CALL  0x04
#define SYSTEM_STATE_RINGING      0x08
#define SYSTEM_STATE_HOLD         0x10

// Declare system_state as volatile (global, accessed by multiple tasks)
extern volatile int32_t system_state;

// Function prototypes
void set_system_state(int32_t new_state);
int32_t get_system_state(void);

#endif // MA_BELL_H
