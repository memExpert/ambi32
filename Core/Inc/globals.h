#ifndef __GLOBALS_H
#define __GLOBALS_H
#include <stdint.h>

extern uint8_t leds_from_usb1[];
extern uint8_t leds_from_usb2[];

extern uint8_t *leds_for_processing;
extern uint8_t *leds_for_writing   ;
extern uint8_t *temp_led_ptr       ;

extern uint8_t data_ready;
#endif