/*
 * Minimal checks for the host tests (one program per module, run by CTest).
 */
#ifndef AMBI_TEST_H
#define AMBI_TEST_H

#include <stdio.h>

static int ambi_test_failures;

#define CHECK(c)                                                              \
    do {                                                                      \
        if (!(c)) {                                                           \
            printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #c);             \
            ambi_test_failures++;                                             \
        }                                                                     \
    } while (0)

static inline int ambi_test_result(const char *name)
{
    printf("%s: %s (%d failures)\n", name, ambi_test_failures ? "FAIL" : "ok", ambi_test_failures);
    return ambi_test_failures ? 1 : 0;
}

#endif /* AMBI_TEST_H */
