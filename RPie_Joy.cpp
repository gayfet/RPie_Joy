#include "bsp/board.h"
#include "tusb.h"
#include <cstdint>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

// --- Hardware Pin Definitions ---
const uint8_t ADC_PIN_X = 26; 
const uint8_t ADC_PIN_Y = 27; 
const uint8_t BUTTON_PINS[6] = {2, 3, 4, 5, 6, 7}; 

// --- Custom HID Report Struct ---
// This strictly maps out the bytes we send to the PC.
// __attribute__((packed)) prevents the C++ compiler from adding empty memory padding.
struct __attribute__((packed)) hotas_report_t {
    uint16_t x;       // 16-bit integer to hold our 12-bit X value
    uint16_t y;       // 16-bit integer to hold our 12-bit Y value
    uint8_t buttons;  // 8-bit integer to hold our 6 buttons + 2 empty padding bits
};

// --- Input Read Functions ---
bool digital_read(uint8_t pin) {
    return !gpio_get(pin);
}

// Now simply returns the raw 12-bit unsigned integer (0-4095)
uint16_t analog_read_axis(uint8_t adc_channel) {
    adc_select_input(adc_channel);
    return adc_read(); 
}

int main() {
    board_init();
    tusb_init();

    stdio_init_all();
    adc_init();

    adc_gpio_init(ADC_PIN_X);
    adc_gpio_init(ADC_PIN_Y);

    for (int i = 0; i < 6; i++) {
        gpio_init(BUTTON_PINS[i]);
        gpio_set_dir(BUTTON_PINS[i], GPIO_IN);
        gpio_pull_up(BUTTON_PINS[i]); 
    }

    uint32_t last_report_time = 0;

    while (true) {
        tud_task(); 
        
        uint32_t current_time = board_millis();

        if (tud_hid_ready() && (current_time - last_report_time > 10)) {
            
            // Create our custom report package
            hotas_report_t report = {0};

            // Read the raw 12-bit analog values
            report.x = analog_read_axis(0); 
            report.y = analog_read_axis(1); 

            // Pack the 6 buttons into the 8-bit integer
            for (int i = 0; i < 6; i++) {
                if (digital_read(BUTTON_PINS[i])) {
                    report.buttons |= (1 << i); 
                }
            }

            // Send the raw struct memory directly over USB
            tud_hid_report(0, &report, sizeof(report));

            last_report_time = current_time;
        }
    }
    return 0;
}

// ==========================================
// TinyUSB HID Callbacks (Required for linking)
// ==========================================
extern "C" {
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    (void) instance; (void) report_id; (void) report_type; (void) buffer; (void) reqlen;
    return 0;
}
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    (void) instance; (void) report_id; (void) report_type; (void) buffer; (void) bufsize;
}
} // extern "C"
