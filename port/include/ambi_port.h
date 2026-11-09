/*
 * Hardware contract of the ambi firmware (L1). Implemented by the family
 * port (port/stm32<fam>/) and port/arch/cortexm/.
 */
#ifndef AMBI_PORT_H
#define AMBI_PORT_H

#include <stdbool.h>
#include <stdint.h>

/* Status LED off, clocks (a crystal or PLL that does not start: three LED
 * flashes and a reset; losing the crystal later or a HardFault: reset),
 * vector table, 1 ms tick. First call in main(). */
void ambi_port_init(void);

/* Milliseconds since ambi_port_init(), wraps after ~49 days. */
uint32_t ambi_port_millis(void);

/* The 96-bit unique ID of the chip. */
void ambi_port_uid(uint8_t uid[12]);

void ambi_port_led(bool on);

#endif /* AMBI_PORT_H */
