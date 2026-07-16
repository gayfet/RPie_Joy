#include "bsp/board.h"
#include "tusb.h"
#include "pico/stdlib.h"

int main() {
    // Initialize Pico board and TinyUSB
    board_init();
    tusb_init();

    uint32_t last_action_time = 0;
    bool key_is_pressed = false;

    while (true) {
        // tud_task() MUST be called frequently to handle USB traffic
        tud_task(); 

        // Only try to send keystrokes if the PC has fully recognized us
        if (tud_hid_ready()) {
            uint32_t current_time = board_millis();

            // Every 1000ms, press 'A'
            if (!key_is_pressed && (current_time - last_action_time > 1000)) {
                
                uint8_t keycode[6] = { HID_KEY_A };
                tud_hid_keyboard_report(0, KEYBOARD_MODIFIER_LEFTSHIFT, keycode);
                
                key_is_pressed = true;
                last_action_time = current_time;
            }
            // 50ms after pressing, release the key
            else if (key_is_pressed && (current_time - last_action_time > 50)) {
                
                // Sending a nullptr report releases all keys
                tud_hid_keyboard_report(0, 0, nullptr);
                
                key_is_pressed = false;
                last_action_time = current_time;
            }
        }
    }
    return 0;
}

