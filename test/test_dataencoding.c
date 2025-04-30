#include "../src/dataencoding.h"
#include "../src/segment.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define SPACE "\x81\x40"

static void test_encodeDataCodewords_Numeric(void) {
    const uint8_t *data = (const uint8_t *)"01234567";
    size_t numDataCodewords = 16;
    uint8_t dataCodewords[16];

    Segment *segments = newSegment(MODE_NUMERIC, 8);

    encodeDataCodewords(dataCodewords, numDataCodewords, data, segments,
                        VERSION_CLASS_SMALL);

    const uint8_t expected[] = {0x10, 0x20, 0x0C, 0x56, 0x61, 0x80, 0xEC, 0x11,
                                0xEC, 0x11, 0xEC, 0x11, 0xEC, 0x11, 0xEC, 0x11};

    assert(memcmp(dataCodewords, expected, numDataCodewords) == 0);

    printf("test_encodeDataCodewords_Numeric() passed\n");
}

static void test_encodeDataCodewords_Alphanumeric(void) {
    const uint8_t *data = (const uint8_t *)"HELLO WORLD";
    size_t numDataCodewords = 13;
    uint8_t dataCodewords[13];

    Segment *segments = newSegment(MODE_ALPHANUMERIC, 11);

    encodeDataCodewords(dataCodewords, numDataCodewords, data, segments,
                        VERSION_CLASS_SMALL);

    const uint8_t expected[] = {0x20, 0x5B, 0x0B, 0x78, 0xD1, 0x72, 0xDC,
                                0x4D, 0x43, 0x40, 0xEC, 0x11, 0xEC};

    assert(memcmp(dataCodewords, expected, numDataCodewords) == 0);

    printf("test_encodeDataCodewords_Alphanumeric() passed\n");
}

static void test_encodeDataCodewords_Byte(void) {
    const uint8_t *data = (const uint8_t *)"Hello, world!";
    size_t numDataCodewords = 16;
    uint8_t dataCodewords[16];

    Segment *segments = newSegment(MODE_BYTE, 13);

    encodeDataCodewords(dataCodewords, numDataCodewords, data, segments,
                        VERSION_CLASS_SMALL);

    const uint8_t expected[] = {0x40, 0xD4, 0x86, 0x56, 0xC6, 0xC6, 0xF2, 0xC2,
                                0x07, 0x76, 0xF7, 0x26, 0xC6, 0x42, 0x10, 0xEC};

    assert(memcmp(dataCodewords, expected, numDataCodewords) == 0);

    printf("test_encodeDataCodewords_Byte() passed\n");
}

static void test_encodeDataCodewords_Kanji(void) {
    const uint8_t data[] = {0xE9, 0xB3, 0x96, 0xA3, 0xE9, 0xB1, 0xE9, 0xB2};
    size_t numDataCodewords = 9;
    uint8_t dataCodewords[9];

    Segment *segments = newSegment(MODE_KANJI, 8);

    encodeDataCodewords(dataCodewords, numDataCodewords, data, segments,
                        VERSION_CLASS_SMALL);

    const uint8_t expected[] = {0x80, 0x4F, 0x39, 0xC0, 0x8F,
                                0xCE, 0x3E, 0x72, 0x00};

    assert(memcmp(dataCodewords, expected, numDataCodewords) == 0);

    printf("test_encodeDataCodewords_Kanji() passed\n");
}

static void test_encodeDataCodewords_Mixed(void) {
    const uint8_t *data = (const uint8_t *)"12345FGHIjkl" SPACE;
    size_t numDataCodewords = 16;
    uint8_t dataCodewords[16];

    Segment *segments = NULL;
    segments = addSegment(segments, newSegment(MODE_NUMERIC, 5));
    segments = addSegment(segments, newSegment(MODE_ALPHANUMERIC, 4));
    segments = addSegment(segments, newSegment(MODE_BYTE, 3));
    segments = addSegment(segments, newSegment(MODE_KANJI, 2));

    encodeDataCodewords(dataCodewords, numDataCodewords, data, segments,
                        VERSION_CLASS_SMALL);

    const uint8_t expected[] = {0x10, 0x14, 0x7B, 0x5A, 0x40, 0x45, 0x66, 0xC3,
                                0xD0, 0x0D, 0xA9, 0xAD, 0xB2, 0x00, 0x40, 0x00};

    assert(memcmp(dataCodewords, expected, numDataCodewords) == 0);

    printf("test_encodeDataCodewords_Mixed() passed\n");
}

int main(void) {
    test_encodeDataCodewords_Numeric();
    test_encodeDataCodewords_Alphanumeric();
    test_encodeDataCodewords_Byte();
    test_encodeDataCodewords_Kanji();
    test_encodeDataCodewords_Mixed();

    return 0;
}
