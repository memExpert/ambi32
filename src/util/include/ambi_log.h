/*
 * Debug log: printf (newlib-nano) into a ring buffer drained by the UART
 * interrupt, never blocking; bytes that do not fit are dropped.
 * Compiled in Debug builds only (AMBI_LOG = 1); otherwise AMBI_LOGF() and
 * ambi_log_init() vanish.
 */
#ifndef AMBI_LOG_H
#define AMBI_LOG_H

#include <stdint.h>

#if AMBI_LOG
#include <stdio.h>
void ambi_log_init(void);
#define AMBI_LOGF(...) printf(__VA_ARGS__)
#else
static inline void ambi_log_init(void) {}
#define AMBI_LOGF(...) ((void)0)
#endif

#endif /* AMBI_LOG_H */
