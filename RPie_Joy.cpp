#include "bsp/board.h"
#include "tusb.h"
#include <cstdint>

int main() {
    // Initialize Pico board and TinyUSB
    board_init();
    tusb_init();

    while (true) {
        // tud_task() MUST be called frequently to handle USB traffic.
        // It keeps the connection to the host PC alive.
        tud_task(); 
        // All key press simulation logic has been removed.
    }
    return 0;
}

// ==========================================
// TinyUSB HID Callbacks (Required for linking)
// ==========================================
extern "C" {

// Invoked when received GET_REPORT control request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    // We do not currently support any get_report requests
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;
    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint (e.g. Keyboard LED states)
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    // We do not currently support any set_report requests
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;
}

} // extern "C"