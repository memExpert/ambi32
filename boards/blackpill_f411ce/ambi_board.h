/*
 * WeAct Black Pill V3.0 (STM32F411CEU6): 512 KiB flash, 128 KiB SRAM,
 * 25 MHz crystal, LED PC13 (lit low), KEY PA0 (reserved for umcub entry).
 * Board configuration: #defines only, read by the port and the firmware.
 */
#ifndef AMBI_BOARD_H
#define AMBI_BOARD_H

#include "ambi_pin.h"

#define AMBI_BOARD_NAME          "blackpill_f411ce"

#define AMBI_BOARD_HSE_HZ        25000000u

/* Application image: the whole flash while there is no bootloader (MVP);
 * with umcub this becomes the primary slot. */
#define AMBI_BOARD_APP_ADDR      0x08000000
#define AMBI_BOARD_APP_SIZE      0x80000
#define AMBI_BOARD_RAM_ADDR      0x20000000
#define AMBI_BOARD_RAM_SIZE      0x20000

#define AMBI_BOARD_LED           AMBI_PIN('C', 13, 0)
#define AMBI_BOARD_LED_ACTIVE_LOW 1

/* Debug log (Debug builds only): USART1 TX on PA9, AF7. */
#define AMBI_BOARD_LOG_USART     1
#define AMBI_BOARD_LOG_TX        AMBI_PIN('A', 9, 7)
#define AMBI_BOARD_LOG_BAUD      115200u

#endif /* AMBI_BOARD_H */
