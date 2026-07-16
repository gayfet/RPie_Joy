#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

// Configure TinyUSB to run in device mode
#define CFG_TUSB_RHPORT0_MODE   OPT_MODE_DEVICE

// Enable HID driver
#define CFG_TUD_HID             1

// Endpoint sizes
#define CFG_TUD_ENDPOINT0_SIZE  64
#define CFG_TUD_HID_EP_BUFSIZE  16

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */