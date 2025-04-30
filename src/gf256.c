/*
 * Copyright 2025 Naoto Yoshida
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "gf256.h"
#include <string.h>

// used to compute the powers of the primitive element 2 under GF(2^8)
static uint8_t expTable[256];

// used to compute the primitive element 2 under GF(2^8)
static uint8_t logTable[256];

/**
 * Initialize the logarithm and exponential tables for GF(2^8).
 */
void gf256_initialize(void) {
    unsigned int x = 1;

    for (size_t i = 0; i < 255; i++) {
        expTable[i] = x;
        logTable[x] = i;
        x <<= 1;

        if (x & 0x100) {
            x ^= 0x11D;
        }
    }

    expTable[255] = 1;
}

/**
 * Initialize the generator polynomial for Reed-Solomon encoding.
 *
 * @param polynomial The generator polynomial. Stored in reverse order
 * @param degree The degree of the generator polynomial
 */
void gf256_initializeGeneratorPolynomial(uint8_t *polynomial, size_t degree) {

    // compute (x - a^i) * (previous generator polynomial)
    // e.g. (x - a^2) * (x^2 + a^25x + a^1)
    // = x^3 + a^25x^2 + a^1x - a^2x^2 - a^27x - a^3
    // = x^3 + (a^25 + a^2)x^2 + (a^1 + a^27)x + a^3
    // = x^3 + (   3 XOR 4)x^2 + (  2 XOR 12)x + a^3
    // = x^3 +            7x^2 +           14x + a^3
    // = x^3 +        a^198x^2 +        a^199x + a^3
    for (size_t i = 1; i < degree; i++) {
        for (size_t j = i; j > 0; j--) {
            polynomial[j] = logTable[expTable[polynomial[j - 1]] ^
                                     expTable[(polynomial[j] + i) % 255]];
        }

        polynomial[0] = (polynomial[0] + i) % 255;
    }
}

/**
 * Divide the message polynomial by the generator polynomial.
 *
 * @param remainder The remainder of the division
 * @param messagePoly The dividend polynomial
 * @param messagePolyLength The length of the message polynomial
 * @param generatorPoly The divisor polynomial
 * @param generatorPolyLength The length of the generator polynomial
 */
void gf256_divideByGeneratorPolynomial(uint8_t *remainder,
                                       const uint8_t *messagePoly,
                                       size_t messagePolyLength,
                                       const uint8_t *generatorPoly,
                                       size_t generatorPolyLength) {
    uint8_t buffer[256] = {0};

    memcpy(buffer, messagePoly, messagePolyLength);

    // long division in GF(2^8) is similar to long division in integers:
    // 1. multiply the generator polynomial by the factor
    // 2. XOR the result of step 1 with the dividend in integer notation
    // 3. store the result of step 2 as the remainder, or the new dividend,
    //    in the buffer[i + 1] to [i + generatorPolyLength]
    // 4. repeat steps 1-3 until the dividend is zero
    for (size_t i = 0; i < messagePolyLength; i++) {
        if (buffer[i]) {

            // most significant digit of dividend in alpha notation
            uint8_t factor = logTable[buffer[i]];

            for (size_t j = 1; j <= generatorPolyLength; j++) {
                buffer[i + j] ^=
                    expTable[(factor + generatorPoly[generatorPolyLength - j]) %
                             255];
            }
        }
    }

    memcpy(remainder, buffer + messagePolyLength, generatorPolyLength);
}
