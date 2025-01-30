#include "globals.h"

uint8_t leds_from_usb1[132 * 3] = {0,};
uint8_t leds_from_usb2[132 * 3] = {0,};

uint8_t *leds_for_processing = leds_from_usb1;
uint8_t *leds_for_writing    = leds_from_usb2;
uint8_t *temp_led_ptr        = 0;

uint8_t data_ready = 0;