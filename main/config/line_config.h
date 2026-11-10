#ifndef __LINE_CONFIG_H__
#define __LINE_CONFIG_H__

// The central office's timers for a handset that is off the hook but not in
// a call (main/app/line/line.c).

// Dial tone plays this long with nothing dialed, then reorder
#define LINE_DIAL_TONE_TIMEOUT_MS     15000

// Reorder plays this long, then the off-hook warning (howler) until on-hook
#define LINE_REORDER_TIMEOUT_MS       30000

// After the far end hangs up, silence this long, then dial tone again
#define LINE_CALL_ENDED_SILENCE_MS    10000

// Line task
#define LINE_TASK_STACK_SIZE          3072
#define LINE_TASK_PRIORITY            6
#define LINE_EVENT_QUEUE_LEN          8

#endif /* __LINE_CONFIG_H__ */
