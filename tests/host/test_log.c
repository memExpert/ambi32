/*
 * Host test of src/util/ambi_log.c: _write() fills the ring, the UART
 * interrupt side drains it through ambi_log_tx_byte(), overflow drops bytes.
 */
#include "ambi_port_log.h"
#include "ambi_test.h"

int _write(int fd, const char *buf, int len);

static int kicks;

/* Port stubs. */
void ambi_port_log_init(void)
{
}

void ambi_port_log_kick(void)
{
    kicks++;
}

static void test_write_then_drain(void)
{
    kicks = 0;
    CHECK(_write(1, "hi\n", 3) == 3);
    CHECK(kicks == 1);
    CHECK(ambi_log_tx_byte() == 'h');
    CHECK(ambi_log_tx_byte() == 'i');
    CHECK(ambi_log_tx_byte() == '\n');
    CHECK(ambi_log_tx_byte() == -1);
}

/* Never blocks: a full ring drops the rest, _write still reports success. */
static void test_overflow_drops(void)
{
    static char big[1500];
    for (unsigned i = 0; i < sizeof big; i++) {
        big[i] = (char)('a' + i % 26);
    }
    CHECK(_write(1, big, (int)sizeof big) == (int)sizeof big);
    int n = 0;
    while (ambi_log_tx_byte() >= 0) {
        n++;
    }
    CHECK(n == 1024);
}

int main(void)
{
    test_write_then_drain();
    test_overflow_drops();
    return ambi_test_result("log");
}
