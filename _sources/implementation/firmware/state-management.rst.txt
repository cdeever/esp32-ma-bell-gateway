State Management
================

Overview
--------

The Ma Bell Gateway uses a centralized state management system that provides efficient state tracking and task synchronization. The system is designed around bitmasks for state representation and FreeRTOS task notifications for state change awareness.

Key Features
------------

1. Bitmask-based State Representation
   - Each state category (phone, bluetooth, network, system) uses an 8-bit mask
   - Each bit represents a specific state condition
   - Multiple states can be active simultaneously

2. State Change Notifications
   - Tasks can register to receive notifications when specific states change
   - Uses FreeRTOS task notifications for efficient task synchronization
   - No polling required - tasks sleep until state changes

State Categories
----------------

1. Phone State (8 bits)
   - Off hook status
   - Ringing status
   - Dialing status
   - Various tone states

2. Bluetooth State (8 bits)
   - Connection status
   - Call status
   - Audio status
   - Mute states

3. Network State (8 bits)
   - WiFi connection
   - IP acquisition
   - DNS status
   - Web server status

4. System State (8 bits)
   - Initialization status
   - Error conditions
   - Battery status
   - System modes

Thread Safety
-------------

The state engine is designed for use in a multi-threaded environment but requires careful consideration of thread safety. Here are the key points and patterns:

State Reading
~~~~~~~~~~~~~

Reading state is thread-safe for individual operations, but when multiple related states need to be read together, use a critical section:

.. code-block:: c

    // Example of safe multi-state reading
    TaskHandle_t current_task = xTaskGetCurrentTaskHandle();
    vTaskSuspendAll();  // Prevent context switches
    {
        bool is_off_hook = ma_bell_state_phone_bits_set(PHONE_STATE_OFF_HOOK);
        bool is_ringing = ma_bell_state_phone_bits_set(PHONE_STATE_RINGING);
        // Use both values together...
    }
    xTaskResumeAll();

State Updates
~~~~~~~~~~~~~

State updates are not atomic - use FreeRTOS primitives for synchronization. For coordinated updates, use a mutex:

.. code-block:: c

    // Example of safe state updates
    static SemaphoreHandle_t state_mutex = NULL;
    
    void update_phone_state(void) {
        if (xSemaphoreTake(state_mutex, portMAX_DELAY) == pdTRUE) {
            // Update multiple related states
            ma_bell_state_update_phone_bits(PHONE_STATE_OFF_HOOK, 0);
            ma_bell_state_update_phone_bits(0, PHONE_STATE_RINGING);
            xSemaphoreGive(state_mutex);
        }
    }

Notification Handling
~~~~~~~~~~~~~~~~~~~~~

Notifications are delivered asynchronously. Handle them promptly to prevent task starvation and use timeouts to prevent deadlocks:

.. code-block:: c

    // Example of safe notification handling
    while (1) {
        uint32_t notification = ma_bell_state_wait_for_notification(
            NOTIFY_PHONE_STATE_CHANGED | NOTIFY_BT_STATE_CHANGED,
            pdMS_TO_TICKS(1000)  // 1 second timeout
        );
        
        if (notification == 0) {
            // Handle timeout
            continue;
        }
        
        // Process notifications
        if (notification & NOTIFY_PHONE_STATE_CHANGED) {
            // Handle phone state change
        }
    }

Critical Sections
~~~~~~~~~~~~~~~~~

- Keep critical sections as short as possible
- Don't call blocking functions inside critical sections
- Consider using task notifications for synchronization

Common Pitfalls
~~~~~~~~~~~~~~~

- Don't assume state hasn't changed between reads
- Don't hold locks while waiting for notifications
- Don't update state from ISRs (use task notifications)
- Don't block in notification handlers

ISR Safety
~~~~~~~~~~

State updates from ISRs should use task notifications:

.. code-block:: c

    // In ISR
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(state_update_task, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    
    // In task
    void state_update_task(void *pvParameters) {
        while (1) {
            if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY)) {
                // Safe to update state here
                ma_bell_state_update_phone_bits(PHONE_STATE_OFF_HOOK, 0);
            }
        }
    }

Best Practices
--------------

1. Always initialize the state engine before use
2. Use the provided bit manipulation functions instead of direct access
3. Register for notifications early in task initialization
4. Handle state changes promptly to maintain system responsiveness
5. Use appropriate timeouts when waiting for notifications
6. Keep critical sections as short as possible
7. Use mutexes for coordinated state updates
8. Handle ISR state updates through task notifications 