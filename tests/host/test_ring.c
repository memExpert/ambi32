/*
 * Host test of src/util/ambi_ring.c.
 */

#include "ambi_ring.h"
#include "ambi_test.h"

static void test_empty(void)
{
    uint8_t mem[8], b;
    ambi_ring_t r;
    ambi_ring_init(&r, mem, sizeof mem);
    CHECK(ambi_ring_used(&r) == 0);
    CHECK(!ambi_ring_get(&r, &b));
}

static void test_fifo_order(void)
{
    uint8_t mem[8], b;
    ambi_ring_t r;
    ambi_ring_init(&r, mem, sizeof mem);
    CHECK(ambi_ring_put(&r, (const uint8_t *)"abc", 3) == 3);
    CHECK(ambi_ring_used(&r) == 3);
    CHECK(ambi_ring_get(&r, &b) && b == 'a');
    CHECK(ambi_ring_get(&r, &b) && b == 'b');
    CHECK(ambi_ring_get(&r, &b) && b == 'c');
    CHECK(!ambi_ring_get(&r, &b));
}

static void test_full_drops_tail(void)
{
    uint8_t mem[8], b;
    ambi_ring_t r;
    ambi_ring_init(&r, mem, sizeof mem);
    CHECK(ambi_ring_put(&r, (const uint8_t *)"0123456789", 10) == 8);
    CHECK(ambi_ring_used(&r) == 8);
    CHECK(ambi_ring_put(&r, (const uint8_t *)"x", 1) == 0);
    for (char c = '0'; c <= '7'; c++) {
        CHECK(ambi_ring_get(&r, &b) && b == (uint8_t)c);
    }
    CHECK(!ambi_ring_get(&r, &b));
}

/* Indices run past 2^32 - n and the buffer end many times. */
static void test_wrap(void)
{
    uint8_t mem[4], b;
    ambi_ring_t r;
    ambi_ring_init(&r, mem, sizeof mem);
    r.head = r.tail = 0xFFFFFFFEu;
    for (unsigned i = 0; i < 100; i++) {
        uint8_t in[3] = {(uint8_t)i, (uint8_t)(i + 1), (uint8_t)(i + 2)};
        CHECK(ambi_ring_put(&r, in, 3) == 3);
        CHECK(ambi_ring_used(&r) == 3);
        for (unsigned k = 0; k < 3; k++) {
            CHECK(ambi_ring_get(&r, &b) && b == in[k]);
        }
    }
    CHECK(ambi_ring_used(&r) == 0);
}

static void test_static_init_and_size_check(void)
{
    static uint8_t mem[16];
    ambi_ring_t r = AMBI_RING_INIT(mem);
    uint8_t b;
    CHECK(ambi_ring_put(&r, (const uint8_t *)"z", 1) == 1);
    CHECK(ambi_ring_get(&r, &b) && b == 'z');
    CHECK(AMBI_RING_SIZE_OK(2u) && AMBI_RING_SIZE_OK(1024u));
    CHECK(!AMBI_RING_SIZE_OK(0u) && !AMBI_RING_SIZE_OK(1u) && !AMBI_RING_SIZE_OK(1000u));
}

int main(void)
{
    test_static_init_and_size_check();
    test_empty();
    test_fifo_order();
    test_full_drops_tail();
    test_wrap();
    return ambi_test_result("ring");
}
