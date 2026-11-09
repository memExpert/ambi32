/*
 * ambi firmware entry: super-loop. Step 1 of the MVP: clocks, tick, status
 * LED blinking at 1 Hz, boot line on the debug log.
 */
#include "ambi_board.h"
#include "ambi_log.h"
#include "ambi_port.h"

#define BLINK_HALF_PERIOD_MS 500u

int main(void)
{
    ambi_port_init();
    ambi_log_init();

    uint8_t uid[12];
    ambi_port_uid(uid);
    AMBI_LOGF("ambi " AMBI_BOARD_NAME " uid ");
    for (unsigned i = 0; i < sizeof uid; i++) {
        AMBI_LOGF("%02x", uid[i]);
    }
    AMBI_LOGF("\r\n");

    uint32_t last = ambi_port_millis();
    bool led = false;
    for (;;) {
        uint32_t now = ambi_port_millis();
        if ((uint32_t)(now - last) >= BLINK_HALF_PERIOD_MS) {
            last = now;
            led = !led;
            ambi_port_led(led);
        }
    }
}
