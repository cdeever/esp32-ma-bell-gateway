#ifndef __TONE_TEST_H__
#define __TONE_TEST_H__

#include <stdbool.h>

/**
 * @brief Bench test: play call-progress tones on command from the cell phone
 *
 * Until off-hook detection exists, the tones are started by dialing a short
 * code on the paired cell phone. The gateway sees the outgoing call's number
 * (from AT+CLCC), hangs the call up while it is still dialing, and plays the
 * tone on the I2S output:
 *
 *   *1 dial tone          *4 reorder (fast busy)    *9 run every tone in turn
 *   *2 ringback           *5 off-hook warning       *0 stop
 *   *3 busy signal        *6 stutter dial tone      *7 / *8 quieter / louder (3 dB)
 *
 * Enabled by CONFIG_MA_BELL_TONE_TEST.
 *
 * @param number The number the phone is dialing
 * @return true if the number was a test code and was handled (the caller
 *         should hang the call up), false otherwise
 */
bool tone_test_handle_dialed(const char *number);

#endif /* __TONE_TEST_H__ */
