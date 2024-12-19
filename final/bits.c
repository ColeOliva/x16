#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include "bits.h"

// Assert that the argument is a bit of either 0 or 1
#define assert_bit(a) if ((a) != 0 && (a) != 1) { assert(false); }

// Get the nth bit
uint16_t getbit(uint16_t number, int n) {
    assert(n >= 0 && n < 16);
    return (number >> n) & 1;
}

// Get bits that are the given number of bits wide
uint16_t getbits(uint16_t number, int n, int wide) {
    assert(n >= 0 && n < 16 && wide > 0 && wide <= 16);
    return (number >> n) & ((1 << wide) - 1);
}

// Set the nth bit to the given bit value and return the result
uint16_t setbit(uint16_t number, int n) {
    assert(n >= 0 && n < 16);
    return number | (1 << n);
}

// Clear the nth bit
uint16_t clearbit(uint16_t number, int n) {
    assert(n >= 0 && n < 16);
    return number & ~(1 << n);
}

// Sign extend a number of the given bits to 16 bits
uint16_t sign_extend(uint16_t x, int bit_count) {
    assert(bit_count >= 0 && bit_count <= 16);
    if (x & (1 << (bit_count - 1))) {
        return x | (0xFFFF << bit_count);
    } else {
        return x & ((1 << bit_count) - 1);
    }
}

bool is_positive(uint16_t number) {
    return getbit(number, 15) == 0;
}

bool is_negative(uint16_t number) {
    return getbit(number, 15) == 1;
}
