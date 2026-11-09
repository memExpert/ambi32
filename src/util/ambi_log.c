/*
 * Debug log (Debug builds only): newlib's _write() puts stdout into a ring
 * buffer, the port's UART interrupt takes the bytes back out through
 * ambi_log_tx_byte(). Thread mode only (single producer): never log from an
 * interrupt.
 */
#include <stdio.h>

#include "ambi_log.h"
#include "ambi_port_log.h"
#include "ambi_ring.h"

#define LOG_RING_SIZE 1024u

_Static_assert(AMBI_RING_SIZE_OK(LOG_RING_SIZE), "LOG_RING_SIZE must be a power of two");

static uint8_t ring_mem[LOG_RING_SIZE];
static ambi_ring_t ring = AMBI_RING_INIT(ring_mem);   /* valid before ambi_log_init() */
static char line_buf[64];

void ambi_log_init(void)
{
    /* Line buffered in a static buffer (no malloc): one _write() per line,
     * not per character. Output without a newline waits for the next one. */
    setvbuf(stdout, line_buf, _IOLBF, sizeof line_buf);
    ambi_port_log_init();
}

int ambi_log_tx_byte(void)
{
    uint8_t b;
    return ambi_ring_get(&ring, &b) ? b : -1;
}

/* newlib system call: stdout and stderr go to the log. */
int _write(int fd, const char *buf, int len);
int _write(int fd, const char *buf, int len)
{
    (void)fd;
    (void)ambi_ring_put(&ring, (const uint8_t *)buf, (size_t)len);   /* what does not fit is dropped */
    ambi_port_log_kick();
    return len;
}
