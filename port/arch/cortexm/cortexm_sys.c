/*
 * Cortex-M system services shared by the family ports: vector table and the
 * SysTick time base. Based on umcub port/arch/cortexm/cortexm_sys.c.
 */
#include "ambi_family_cmsis.h"
#include "ambi_port.h"
#include "cortexm_port.h"

extern const uint32_t g_pfnVectors[];   /* CMSIS startup file */

static volatile uint32_t ticks;

void SysTick_Handler(void)
{
    ticks++;
}

/* Linked with -nostartfiles (the CMSIS startup file replaces newlib's crt0):
 * __libc_init_array() still calls _init(); nothing to do for C. */
void _init(void);
void _init(void)
{
}

/* Faults (MemManage, BusFault and UsageFault escalate here while disabled):
 * reset instead of the startup file's endless default handler. */
void HardFault_Handler(void);
void HardFault_Handler(void)
{
    NVIC_SystemReset();
}

uint32_t ambi_port_millis(void)
{
    return ticks;
}

void ambi_cm_quiesce(void)
{
    __disable_irq();
    SysTick->CTRL = 0;
    for (unsigned i = 0; i < sizeof NVIC->ICER / sizeof NVIC->ICER[0]; i++) {
        NVIC->ICER[i] = 0xFFFFFFFFu;
        NVIC->ICPR[i] = 0xFFFFFFFFu;
    }
    SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk;
}

void ambi_cm_start(uint32_t cpu_hz, uint32_t systick_prio)
{
    /* The image may be linked at a slot address (umcub): its own vectors. */
    SCB->VTOR = (uint32_t)g_pfnVectors;
    __DSB();
    SysTick->LOAD = cpu_hz / 1000u - 1u;
    SysTick->VAL = 0;
    NVIC_SetPriority(SysTick_IRQn, systick_prio);
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
    __enable_irq();
}
