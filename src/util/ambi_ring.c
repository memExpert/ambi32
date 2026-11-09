/*
 * Byte ring buffer (SPSC, lock-free), see ambi_ring.h. The producer writes
 * the data before publishing head, the consumer reads it before releasing
 * tail. buf is not volatile, so signal fences keep the compiler from moving
 * the data accesses across the index updates; on a single core with an
 * interrupt on the other side that is all the ordering needed.
 */
#include <stdatomic.h>

#include "ambi_ring.h"

void ambi_ring_init(ambi_ring_t *r, uint8_t *buf, uint32_t size)
{
    r->buf = buf;
    r->mask = size - 1u;
    r->head = 0;
    r->tail = 0;
}

uint32_t ambi_ring_used(const ambi_ring_t *r)
{
    return r->head - r->tail;
}

size_t ambi_ring_put(ambi_ring_t *r, const uint8_t *data, size_t len)
{
    uint32_t head = r->head;
    uint32_t space = r->mask + 1u - (head - r->tail);
    atomic_signal_fence(memory_order_acquire);   /* slots freed by tail are free now */
    size_t n = len < space ? len : space;
    for (size_t i = 0; i < n; i++) {
        r->buf[(head + i) & r->mask] = data[i];
    }
    atomic_signal_fence(memory_order_release);   /* data before head */
    r->head = head + (uint32_t)n;
    return n;
}

bool ambi_ring_get(ambi_ring_t *r, uint8_t *b)
{
    uint32_t tail = r->tail;
    if (tail == r->head) {
        return false;
    }
    atomic_signal_fence(memory_order_acquire);   /* head before data */
    *b = r->buf[tail & r->mask];
    atomic_signal_fence(memory_order_release);   /* data read before the slot is released */
    r->tail = tail + 1u;
    return true;
}
