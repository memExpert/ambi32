/*
 * Pin encoding shared by board headers and the port: port letter, pin number
 * and alternate function in one constant, e.g. AMBI_PIN('A', 9, 7).
 * No dependencies: board headers include it.
 */
#ifndef AMBI_PIN_H
#define AMBI_PIN_H

#define AMBI_PIN(port, num, af)  ((((port) - 'A') << 16) | ((num) << 8) | (af))
#define AMBI_PIN_PORT(pin)       (((pin) >> 16) & 0xFFu)   /* 0 = A */
#define AMBI_PIN_NUM(pin)        (((pin) >> 8) & 0xFFu)
#define AMBI_PIN_AF(pin)         ((pin) & 0xFFu)

#endif /* AMBI_PIN_H */
