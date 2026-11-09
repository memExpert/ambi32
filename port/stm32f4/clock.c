/*
 * STM32F4 clock tree (RM0383 §6), on LL. SYSCLK 96 MHz from the main PLL fed
 * by the HSE: VCO 192 MHz, /2 for the system, /4 = 48 MHz for USB OTG FS.
 * AHB /1, APB1 /2 (48 MHz), APB2 /1 (96 MHz). 100 MHz and 48 MHz for USB at
 * the same time are not possible. Based on umcub port/stm32f4/clock.c.
 */
#include "f4.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_system.h"

/* PLL input divided down to 2 MHz when the crystal allows it (least jitter),
 * else to 1 MHz; RM0383 §6.3.2: VCO input 1..2 MHz, VCO output 100..432 MHz,
 * PLLM 2..63, PLLN 50..432. */
#define PLL_IN_HZ   (AMBI_BOARD_HSE_HZ % 2000000u == 0 ? 2000000u : 1000000u)
#define PLL_VCO_HZ  (2u * F4_SYSCLK_HZ)

_Static_assert(AMBI_BOARD_HSE_HZ % 1000000u == 0 && AMBI_BOARD_HSE_HZ >= 4000000u &&
               AMBI_BOARD_HSE_HZ <= 26000000u, "HSE must be a whole number of MHz in 4..26 MHz (DS10314)");
_Static_assert(PLL_VCO_HZ / 4u == 48000000u, "USB OTG FS needs 48 MHz from PLLQ = /4");

/* A clock that does not start: three short LED flashes (busy loop on
 * whatever clock runs, ~16 MHz HSI: timing approximate), then a reset - so
 * a board without a working crystal shows a repeating triple flash. */
static void clock_fail(void)
{
    for (unsigned i = 0; i < 6; i++) {
        ambi_port_led(i % 2 == 0);
        for (volatile uint32_t n = 0; n < 400000u; n++) {
        }
    }
    NVIC_SystemReset();
}

/* Waits before SysTick runs: bounded by an iteration count (over 0.5 s at
 * the 16 MHz HSI, against a typical HSE start-up of 2 ms, DS10314 Table 37). */
static void wait_or_fail(uint32_t (*ready)(void))
{
    for (uint32_t n = 0; n < 2000000u; n++) {
        if (ready()) {
            return;
        }
    }
    clock_fail();
}

static uint32_t sysclk_is_pll(void)
{
    return LL_RCC_GetSysClkSource() == LL_RCC_SYS_CLKSOURCE_STATUS_PLL;
}

static uint32_t latency_is_3ws(void)
{
    return LL_FLASH_GetLatency() == LL_FLASH_LATENCY_3;
}

static uint32_t sysclk_is_hsi(void)
{
    return LL_RCC_GetSysClkSource() == LL_RCC_SYS_CLKSOURCE_STATUS_HSI;
}

static uint32_t pll_is_off(void)
{
    return !LL_RCC_PLL_IsReady();
}

void f4_clock_init(void)
{
    /* The ROM DFU bootloader (dfu-util :leave) jumps here with its own clock
     * running: back to the HSI and the PLL off, as after reset; PLLCFGR is
     * writable only with the PLL off (RM0383 §6.3.2). */
    LL_RCC_HSI_Enable();
    wait_or_fail(LL_RCC_HSI_IsReady);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
    wait_or_fail(sysclk_is_hsi);
    LL_RCC_PLL_Disable();
    wait_or_fail(pll_is_off);

    LL_RCC_HSE_Enable();
    wait_or_fail(LL_RCC_HSE_IsReady);
    /* Clock security system: losing the HSE later raises an NMI (RM0383
     * §6.2.7), handled by a reset in NMI_Handler. */
    LL_RCC_HSE_EnableCSS();

    /* Regulator scale 1 for more than 84 MHz (RM0383 §5.4.1); VOS takes
     * effect once the PLL runs. */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);

    /* PLLM is the plain divider value in the field's low bits (LL_RCC_PLLM_DIV_n == n). */
    const uint32_t m = AMBI_BOARD_HSE_HZ / PLL_IN_HZ, n = PLL_VCO_HZ / PLL_IN_HZ;
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, m, n, LL_RCC_PLLP_DIV_2);
    LL_RCC_PLL_ConfigDomain_48M(LL_RCC_PLLSOURCE_HSE, m, n, LL_RCC_PLLQ_DIV_4);
    LL_RCC_PLL_Enable();
    wait_or_fail(LL_RCC_PLL_IsReady);
    wait_or_fail(LL_PWR_IsActiveFlag_VOS);

    /* 3 wait states for 90..100 MHz at 2.7..3.6 V (RM0383 Table 5), set
     * before the clock goes up; prefetch and caches on. */
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);
    wait_or_fail(latency_is_3ws);
    LL_FLASH_EnablePrefetch();
    LL_FLASH_EnableInstCache();
    LL_FLASH_EnableDataCache();

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    wait_or_fail(sysclk_is_pll);
    SystemCoreClock = F4_SYSCLK_HZ;
}

/* NMI: the CSS saw the HSE fail (the hardware already switched SYSCLK to the
 * HSI), or any other NMI source - reset and start over. */
void NMI_Handler(void)
{
    LL_RCC_ClearFlag_HSECSS();
    NVIC_SystemReset();
}
