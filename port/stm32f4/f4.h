/*
 * Private header of the STM32F4 port.
 */
#ifndef AMBI_PORT_F4_H
#define AMBI_PORT_F4_H

#include <stdbool.h>
#include <stdint.h>

#include "ambi_board.h"
#include "ambi_port.h"
#include "stm32f4xx.h"
#include "cortexm_port.h"

/* NVIC priorities (4 bits, 0 = highest). The strip DMA gets the highest
 * priority (docs/architecture.md); the debug log the lowest. */
#define F4_PRIO_SYSTICK     2u
#define F4_PRIO_LOG_UART    15u

/* Clock frequencies after f4_clock_init(). */
#define F4_SYSCLK_HZ        96000000u
#define F4_PCLK2_HZ         F4_SYSCLK_HZ           /* APB2: SPI1/4/5, USART1/6 */

/* SYSCLK 96 MHz from the HSE through the main PLL, 48 MHz for USB, clock
 * security system on. If the HSE or the PLL does not start: three LED
 * flashes, then a reset (the LED must be set up before). */
void f4_clock_init(void);

/* GPIO of an AMBI_PIN(): clock on, the port's registers. */
GPIO_TypeDef *f4_gpio(uint32_t pin);

/* Pin in alternate-function mode (push-pull, no pull, given LL speed). */
void f4_gpio_af(uint32_t pin, uint32_t speed);

#endif /* AMBI_PORT_F4_H */
