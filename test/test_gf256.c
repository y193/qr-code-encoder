#include "gf256.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_gf256_initialize(void) {
    gf256_initialize();

    printf("test_gf256_initialize() passed\n");
}

void test_gf256_initializeGeneratorPolynomial(void) {

    // 2 error correction codewords
    uint8_t *generatorPoly = (uint8_t[2]){0};
    const uint8_t *expected = (const uint8_t[]){1, 25};

    gf256_initializeGeneratorPolynomial(generatorPoly, 2);

    assert(memcmp(generatorPoly, expected, 2) == 0);

    // 3 error correction codewords
    generatorPoly = (uint8_t[3]){0};
    expected = (const uint8_t[]){3, 199, 198};

    gf256_initializeGeneratorPolynomial(generatorPoly, 3);

    assert(memcmp(generatorPoly, expected, 3) == 0);

    // 68 error correction codewords
    generatorPoly = (uint8_t[68]){0};
    expected = (const uint8_t[]){
        238, 163, 8,   5,   3,   127, 184, 101, 27,  235, 238, 43, 198, 175,
        215, 82,  32,  54,  2,   118, 225, 166, 241, 137, 125, 41, 177, 52,
        231, 95,  97,  199, 52,  227, 89,  160, 173, 253, 84,  15, 84,  93,
        151, 203, 220, 165, 202, 60,  52,  133, 205, 190, 101, 84, 150, 43,
        254, 32,  160, 90,  70,  77,  93,  224, 33,  223, 159, 247};

    gf256_initializeGeneratorPolynomial(generatorPoly, 68);

    assert(memcmp(generatorPoly, expected, 68) == 0);

    printf("test_gf256_initializeGeneratorPolynomial() passed\n");
}

void test_gf256_divideByGeneratorPolynomial_Divisible(void) {
    const uint8_t *messagePoly;
    const uint8_t *generatorPoly;
    const uint8_t *expected;
    uint8_t remainder[68];

    generatorPoly = (const uint8_t[]){1, 25};

    messagePoly = (const uint8_t[]){1, 3, 2};
    expected = (const uint8_t[]){0, 0};

    gf256_divideByGeneratorPolynomial(remainder, messagePoly, 3, generatorPoly,
                                      2);

    assert(memcmp(remainder, expected, 2) == 0);

    messagePoly = (const uint8_t[]){1, 2, 1, 2};
    expected = (const uint8_t[]){0, 0};

    gf256_divideByGeneratorPolynomial(remainder, messagePoly, 4, generatorPoly,
                                      2);

    assert(memcmp(remainder, expected, 2) == 0);

    messagePoly = (const uint8_t[]){1, 2, 0, 1, 2};
    expected = (const uint8_t[]){0, 0};

    gf256_divideByGeneratorPolynomial(remainder, messagePoly, 5, generatorPoly,
                                      2);

    assert(memcmp(remainder, expected, 2) == 0);

    printf("test_gf256_divideByGeneratorPolynomial_Divisible() passed\n");
}

void test_gf256_divideByGeneratorPolynomial(void) {
    const uint8_t *messagePoly;
    const uint8_t *generatorPoly;
    const uint8_t *expected;
    uint8_t remainder[68];

    // 2 error correction codewords
    generatorPoly = (const uint8_t[]){1, 25};

    messagePoly = (const uint8_t[]){0};
    expected = (const uint8_t[]){0, 0};

    gf256_divideByGeneratorPolynomial(remainder, messagePoly, 1, generatorPoly,
                                      2);

    assert(memcmp(remainder, expected, 2) == 0);

    messagePoly = (const uint8_t[]){1};
    expected = (const uint8_t[]){3, 2};

    gf256_divideByGeneratorPolynomial(remainder, messagePoly, 1, generatorPoly,
                                      2);

    assert(memcmp(remainder, expected, 2) == 0);

    messagePoly = (const uint8_t[]){0, 0};
    expected = (const uint8_t[]){0, 0};

    gf256_divideByGeneratorPolynomial(remainder, messagePoly, 2, generatorPoly,
                                      2);

    assert(memcmp(remainder, expected, 2) == 0);

    messagePoly = (const uint8_t[]){0, 1};
    expected = (const uint8_t[]){3, 2};

    gf256_divideByGeneratorPolynomial(remainder, messagePoly, 2, generatorPoly,
                                      2);

    assert(memcmp(remainder, expected, 2) == 0);

    messagePoly = (const uint8_t[]){1, 0};
    expected = (const uint8_t[]){7, 6};

    gf256_divideByGeneratorPolynomial(remainder, messagePoly, 2, generatorPoly,
                                      2);

    assert(memcmp(remainder, expected, 2) == 0);

    // 10 error correction codewords
    generatorPoly = (const uint8_t[]){45, 32, 94, 64, 70, 118, 61, 46, 67, 251};

    messagePoly = (const uint8_t[]){16,  32, 12,  86, 97,  128, 236, 17,
                                    236, 17, 236, 17, 236, 17,  236, 17};
    expected = (const uint8_t[]){165, 36, 212, 193, 237, 54, 199, 135, 44, 85};

    gf256_divideByGeneratorPolynomial(remainder, messagePoly, 16, generatorPoly,
                                      10);

    assert(memcmp(remainder, expected, 10) == 0);

    // 13 error correction codewords
    generatorPoly = (const uint8_t[]){78,  140, 206, 218, 130, 104, 106,
                                      100, 86,  100, 176, 152, 74};

    messagePoly = (const uint8_t[]){32, 91, 11, 120, 209, 114, 220,
                                    77, 67, 64, 236, 17,  236};
    expected = (const uint8_t[]){168, 72, 22, 82,  217, 54, 156,
                                 0,   46, 15, 180, 122, 16};

    gf256_divideByGeneratorPolynomial(remainder, messagePoly, 13, generatorPoly,
                                      13);

    assert(memcmp(remainder, expected, 13) == 0);

    messagePoly = (const uint8_t[]){32, 240, 11, 120, 209, 114, 220,
                                    77, 67,  64, 236, 17,  236};
    expected = (const uint8_t[]){146, 204, 111, 143, 33,  22, 186,
                                 54,  199, 231, 243, 170, 255};

    gf256_divideByGeneratorPolynomial(remainder, messagePoly, 13, generatorPoly,
                                      13);

    assert(memcmp(remainder, expected, 13) == 0);

    printf("test_gf256_divideByGeneratorPolynomial() passed\n");
}

int main(void) {
    test_gf256_initialize();
    test_gf256_initializeGeneratorPolynomial();
    test_gf256_divideByGeneratorPolynomial_Divisible();
    test_gf256_divideByGeneratorPolynomial();

    return 0;
}
