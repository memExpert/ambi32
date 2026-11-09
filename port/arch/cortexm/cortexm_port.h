/*
 * Cortex-M services shared by the family ports (CMSIS-Core only). Included
 * by the family's private header after its device header.
 */
#ifndef AMBI_CORTEXM_PORT_H
#define AMBI_CORTEXM_PORT_H

#include <stdint.h>

/* First thing at init: interrupts off, SysTick stopped, every NVIC line
 * disabled and unpended - whoever ran before (ROM DFU bootloader, later
 * umcub) may have left them on. */
void ambi_cm_quiesce(void);

/* Vector table of this image active, SysTick at 1 kHz from `cpu_hz` with
 * priority `systick_prio`, interrupts on. */
void ambi_cm_start(uint32_t cpu_hz, uint32_t systick_prio);

#endif /* AMBI_CORTEXM_PORT_H */
