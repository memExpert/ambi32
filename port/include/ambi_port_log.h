/*
 * Debug log output (Debug builds only): a UART transmitter driven by its
 * TX-empty interrupt. The bytes come from an upcall, so the port keeps no
 * buffer of its own.
 */
#ifndef AMBI_PORT_LOG_H
#define AMBI_PORT_LOG_H

#include <stdint.h>

/* UART of the board (AMBI_BOARD_LOG_*), transmitter only. */
void ambi_port_log_init(void);

/* New bytes are available: start the transmitter if it is idle. Safe to call
 * any time from the main loop. */
void ambi_port_log_kick(void);

/* Upcall from the UART interrupt, implemented by src/util/ambi_log.c: the
 * next byte to send, or -1 when there is none. */
int ambi_log_tx_byte(void);

#endif /* AMBI_PORT_LOG_H */
