#include "MultModulo.h"

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t modulo) {
    uint64_t result = 0;
    a = a % modulo;
    while (b > 0) {
        if (b % 2 == 1)
            result = (result + a) % modulo;
        a = (a * 2) % modulo;
        b /= 2;
    }

    return result % modulo;
}