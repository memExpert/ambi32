/*
 * Byte ring buffer, single producer / single consumer without locks: one
 * side may run in an interrupt. The capacity is a power of two; head and
 * tail run freely and wrap through the mask.
 */
#ifndef AMBI_RING_H
#define AMBI_RING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t *buf;
    uint32_t mask;              /* capacity - 1 */
    volatile uint32_t head;     /* written by the producer only */
    volatile uint32_t tail;     /* written by the consumer only */
} ambi_ring_t;

#define AMBI_RING_SIZE_OK(size) ((size) >= 2u && ((size) & ((size) - 1u)) == 0)

/* `size` must be a power of two (>= 2), see AMBI_RING_SIZE_OK. */
void ambi_ring_init(ambi_ring_t *r, uint8_t *buf, uint32_t size);

/* Static initializer, for a ring usable before any code runs. */
#define AMBI_RING_INIT(mem) {(mem), sizeof(mem) - 1u, 0, 0}

/* Producer: copies as many bytes as fit, returns how many were taken. */
size_t ambi_ring_put(ambi_ring_t *r, const uint8_t *data, size_t len);

/* Consumer: takes one byte into *b; false when empty. */
bool ambi_ring_get(ambi_ring_t *r, uint8_t *b);

uint32_t ambi_ring_used(const ambi_ring_t *r);

#endif /* AMBI_RING_H */
