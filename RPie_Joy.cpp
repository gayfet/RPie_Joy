#include "bsp/board.h"
#include "tusb.h"
#include <cstdint>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

// ==========================================
// HOTAS CONFIGURATION (Must match descriptors)
// ==========================================
#define NUM_AXES 2       // Quantity of analog axes 
#define NUM_BUTTONS 6    // Quantity of buttons (Max 32)

// Map your GPIO pins sequentially. 
// Note: Pico ADC pins must be 26, 27, 28, or 29.
const uint8_t AXIS_PINS[NUM_AXES] = {26, 27}; 
const uint8_t BUTTON_PINS[NUM_BUTTONS] = {2, 3, 4, 5, 6, 7}; 

// --- Custom HID Report Struct ---
// Dynamically sizes the axes array.
// Buttons use a 32-bit int, allowing up to 32 buttons without struct changes.
// --- Custom HID Report Struct ---
struct __attribute__((packed)) hotas_report_t {
    int16_t axes[NUM_AXES];  // CRITICAL FIX: Changed to signed 16-bit
    uint32_t buttons;        
};

// --- Input Read Functions ---
bool digital_read(uint8_t pin) {
    return !gpio_get(pin);
}

// Stretches the 12-bit ADC (0 to 4095) across standard signed 16-bit (-32767 to +32767)
int16_t analog_read_axis(uint8_t adc_channel) {
    adc_select_input(adc_channel);
    uint16_t raw_adc = adc_read(); 
    
    // Smoothly map the 0-4095 hardware read to the full Windows axis range
    int32_t mapped = ((int32_t)raw_adc * 65534 / 4095) - 32767;
    return (int16_t)mapped;
}

int main() {
    board_init();
    tusb_init();

    adc_init();

    //Turn LED on to indicate successful boot
    gpio_init(25); 
    gpio_set_dir(25, GPIO_OUT);
    gpio_put(25,1);

    // Dynamically initialize all defined axes
    for (int i = 0; i < NUM_AXES; i++) {
        adc_gpio_init(AXIS_PINS[i]);
    }

    // Dynamically initialize all defined buttons
    for (int i = 0; i < NUM_BUTTONS; i++) {
        gpio_init(BUTTON_PINS[i]);
        gpio_set_dir(BUTTON_PINS[i], GPIO_IN);
        gpio_pull_up(BUTTON_PINS[i]); 
    }

    uint32_t last_report_time = 0;

    while (true) {
        tud_task(); 
        
        uint32_t current_time = board_millis();

        if (tud_hid_ready() && (current_time - last_report_time > 10)) {
            
            hotas_report_t report = {0};

            // --- COMMENT OUT THE REAL HARDWARE READ ---
            // for (int i = 0; i < NUM_AXES; i++) {
            //     report.axes[i] = analog_read_axis(AXIS_PINS[i] - 26); 
            // }

            // --- INJECT FAKE SWEEPING DATA ---
            static uint16_t sweep_val = 0;
            static bool sweep_up = true;
            
            if (sweep_up) {
                sweep_val += 20;
                if (sweep_val >= 4095) sweep_up = false;
            } else {
                sweep_val -= 20;
                if (sweep_val <= 0) sweep_up = true;
            }

            report.axes[0] = sweep_val; // X axis sweeps left and right
            report.axes[1] = 2047;      // Y axis stays locked dead center

            // Pack the defined buttons into the 32-bit integer
            for (int i = 0; i < NUM_BUTTONS; i++) {
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