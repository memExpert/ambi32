/*
 * STM32F4 port: init, GPIO helpers, status LED, UID. GPIO helpers based on
 * umcub port/stm32f4/sys.c.
 */
#include "f4.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_utils.h"

GPIO_TypeDef *f4_gpio(uint32_t pin)
{
    unsigned port = AMBI_PIN_PORT(pin);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA << port);  /* reads back: ES0287 delay after enable */
    return (GPIO_TypeDef *)(GPIOA_BASE + port * (GPIOB_BASE - GPIOA_BASE));
}

void f4_gpio_af(uint32_t pin, uint32_t speed)
{
    GPIO_TypeDef *g = f4_gpio(pin);
    uint32_t m = 1u << AMBI_PIN_NUM(pin);
    LL_GPIO_SetPinSpeed(g, m, speed);
    LL_GPIO_SetPinOutputType(g, m, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(g, m, LL_GPIO_PULL_NO);
    if (AMBI_PIN_NUM(pin) < 8) {
        LL_GPIO_SetAFPin_0_7(g, m, AMBI_PIN_AF(pin));
    } else {
        LL_GPIO_SetAFPin_8_15(g, m, AMBI_PIN_AF(pin));
    }
    LL_GPIO_SetPinMode(g, m, LL_GPIO_MODE_ALTERNATE);
}

static GPIO_TypeDef *led_gpio;
#define LED_MASK (1u << AMBI_PIN_NUM(AMBI_BOARD_LED))

void ambi_port_led(bool on)
{
    if (on != (bool)AMBI_BOARD_LED_ACTIVE_LOW) {
        LL_GPIO_SetOutputPin(led_gpio, LED_MASK);
    } else {
        LL_GPIO_ResetOutputPin(led_gpio, LED_MASK);
    }
}

static void led_init(void)
{
    GPIO_TypeDef *g = led_gpio = f4_gpio(AMBI_BOARD_LED);
    const uint32_t m = LED_MASK;
    ambi_port_led(false);
    LL_GPIO_SetPinOutputType(g, m, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinSpeed(g, m, LL_GPIO_SPEED_FREQ_LOW);
    LL_GPIO_SetPinPull(g, m, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinMode(g, m, LL_GPIO_MODE_OUTPUT);
}

void ambi_port_init(void)
{
    ambi_cm_quiesce();
    led_init();                 /* first: the clock failure path flashes it */
    f4_clock_init();
    ambi_cm_start(F4_SYSCLK_HZ, F4_PRIO_SYSTICK);
}

void ambi_port_uid(uint8_t uid[12])
{
    const uint32_t w[3] = {LL_GetUID_Word0(), LL_GetUID_Word1(), LL_GetUID_Word2()};
    for (unsigned i = 0; i < 12; i++) {
        uid[i] = (uint8_t)(w[i / 4] >> (8 * (i % 4)));
    }
}
