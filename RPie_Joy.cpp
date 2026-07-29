/*
Gayfet July 2026 
This is a custom implementation of a USB HID device for the Rasberry Pi Pico, designed to function as a joystick with analog axes 
and multiple buttons. It utilizes the TinyUSB stack to handle USB communication and HID report generation. 
*/

#include "bsp/board.h"
#include "tusb.h"
#include <cstdint>
#include <bitset>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

// Hardware pin definitions----
const uint8_t ADC_PIN_X = 26; 
const uint8_t ADC_PIN_Y = 27; 
const uint8_t ADC_PIN_Z = 28;  
const uint8_t ADC_PIN_RZ = 29;

const uint8_t NUM_BUTTONS = 64; 

// --- Custom HID Report Struct ---
// __attribute__((packed)) ensures that the struct is packed without any padding, crucial for HID report. 
struct __attribute__((packed)) hotas_report_t {
    uint16_t x;       // 16-bit integer for X axis
    uint16_t y;       // 16-bit integer for Y axis
    uint16_t z;       // 16-bit integer for Z axis
    uint16_t rz;      // 16-bit integer for RZ axis
    uint64_t button_array; // 64-bit integer to hold exactly 64 buttons (no padding needed)
};

// --- Input Read Functions ---
uint16_t analog_read_axis(uint8_t adc_channel) {
    adc_select_input(adc_channel);
    return adc_read(); 
}

void setButton(uint64_t &buttonArray, uint8_t buttonIndex, bool value) {
    if (buttonIndex >= NUM_BUTTONS) return; // Out of bounds check
    if (value) { //setting button to 1 (true)
        buttonArray |= (1ULL << buttonIndex); // Set the bit
    } else { //setting button to 0 (false)
        buttonArray &= ~(1ULL << buttonIndex); // Clear the bit
    }
}

int main() {
    board_init();
    tusb_init();

    stdio_init_all();
    adc_init();

    // Initialize all 4 ADCs
    adc_gpio_init(ADC_PIN_X);
    adc_gpio_init(ADC_PIN_Y);
    adc_gpio_init(ADC_PIN_Z);
    adc_gpio_init(ADC_PIN_RZ);

    uint32_t last_report_time = 0;

    while (true) {
        tud_task(); // Must be called frequently to maintain USB connection
        
        uint32_t current_time = board_millis();

        if (tud_hid_ready() && (current_time - last_report_time > 10)) {
            
            // Create our custom report package
            hotas_report_t report = {0};

            // Read the raw 12-bit analog values
            report.x = analog_read_axis(0); 
            report.y = analog_read_axis(1); 
            report.z = analog_read_axis(2); 
            report.rz = analog_read_axis(3); 

            setButton(report.button_array, 0, 1); // Sets button 0 to pressed (1)

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