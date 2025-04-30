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

#include "dataencoding.h"
#include "charset.h"
#include <string.h>

#define numBitsModeIndicator 4

static const size_t numBitsCharCountIndicator[][4] = {
    {10, 9, 8, 8}, {12, 11, 16, 10}, {14, 13, 16, 12}};

static const size_t numDataCodewords[][4] = {
    {19, 16, 13, 9},          {34, 28, 22, 16},
    {55, 44, 34, 26},         {80, 64, 48, 36},
    {108, 86, 62, 46},        {136, 108, 76, 60},
    {156, 124, 88, 66},       {194, 154, 110, 86},
    {232, 182, 132, 100},     {274, 216, 154, 122},
    {324, 254, 180, 140},     {370, 290, 206, 158},
    {428, 334, 244, 180},     {461, 365, 261, 197},
    {523, 415, 295, 223},     {589, 453, 325, 253},
    {647, 507, 367, 283},     {721, 563, 397, 313},
    {795, 627, 445, 341},     {861, 669, 485, 385},
    {932, 714, 512, 406},     {1006, 782, 568, 442},
    {1094, 860, 614, 464},    {1174, 914, 664, 514},
    {1276, 1000, 718, 538},   {1370, 1062, 754, 596},
    {1468, 1128, 808, 628},   {1531, 1193, 871, 661},
    {1631, 1267, 911, 701},   {1735, 1373, 985, 745},
    {1843, 1455, 1033, 793},  {1955, 1541, 1115, 845},
    {2071, 1631, 1171, 901},  {2191, 1725, 1231, 961},
    {2306, 1812, 1286, 986},  {2434, 1914, 1354, 1054},
    {2566, 1992, 1426, 1096}, {2702, 2102, 1502, 1142},
    {2812, 2216, 1582, 1222}, {2956, 2334, 1666, 1276}};

static size_t getNumBitsCharCountIndicator(VersionClass versionClass,
                                           Mode mode) {
    // {1, 2, 4, 8} -> {0, 1, 2, 3}
    return numBitsCharCountIndicator[versionClass][(mode ^ -2) / -3];
}

static size_t getNumDataCodewords(unsigned int version,
                                  ErrorCorrectionLevel ecLevel) {
    return numDataCodewords[version - 1][ecLevel];
}

static size_t getNumBitsEncodedData(Mode mode, size_t length) {
    switch (mode) {
    case MODE_NUMERIC:
        return (length / 3 * 10) + (length % 3 / 2 * 7) + (length % 3 % 2 * 7);

    case MODE_ALPHANUMERIC:
        return (length / 2 * 11) + (length % 2 * 6);

    case MODE_BYTE:
        return length * 8;

    case MODE_KANJI:
        return length / 2 * 13;

    default:
        return 0;
    }
}

/**
 * Recommend the smallest version that can contain the given data.
 *
 * @param segments The segments of the data
 * @param ecLevel The error correction level
 * @param versionClass The version class
 * @return The recommended version number
 */
int recommendVersion(const Segment *segments, ErrorCorrectionLevel ecLevel,
                     VersionClass versionClass) {
    size_t numBits = 0;

    for (const Segment *segment = segments; segment != NULL;
         segment = segment->next) {
        numBits += numBitsModeIndicator;
        numBits += getNumBitsCharCountIndicator(versionClass, segment->mode);
        numBits += getNumBitsEncodedData(segment->mode, segment->length);
    }

    unsigned int start = (const unsigned int[]){1, 10, 27}[versionClass];
    unsigned int end = (const unsigned int[]){9, 26, 40}[versionClass];

    for (unsigned int version = start; version <= end; version++) {
        if (numBits <= getNumDataCodewords(version, ecLevel) * 8) {
            return version;
        }
    }

    return -1;
}

static void appendBits(uint8_t *array, size_t *index, unsigned int value,
                       size_t numBits) {
    *index += numBits;

    for (size_t j = 1; j <= numBits; j++) {
        size_t i = *index - j;
        array[i / 8] |= (value & 1) << (7 - i % 8);
        value >>= 1;
    }
}

static void appendModeIndicator(uint8_t *codewords, size_t *index, Mode mode) {
    appendBits(codewords, index, mode, numBitsModeIndicator);
}

static void appendCharCountIndicator(uint8_t *codewords, size_t *index,
                                     size_t charCount,
                                     VersionClass versionClass, Mode mode) {
    appendBits(codewords, index, charCount,
               getNumBitsCharCountIndicator(versionClass, mode));
}

static void appendNumeric(uint8_t *codewords, size_t *index,
                          const uint8_t *data, size_t length) {
    size_t i = 0;

    while (length >= 3) {
        unsigned int value = (data[i] - '0') * 100 + (data[i + 1] - '0') * 10 +
                             (data[i + 2] - '0');
        appendBits(codewords, index, value, 10);
        i += 3;
        length -= 3;
    }

    if (length == 2) {
        unsigned int value = (data[i] - '0') * 10 + (data[i + 1] - '0');
        appendBits(codewords, index, value, 7);

    } else if (length == 1) {
        unsigned int value = data[i] - '0';
        appendBits(codewords, index, value, 4);
    }
}

static void appendAlphanumeric(uint8_t *codewords, size_t *index,
                               const uint8_t *data, size_t length) {
    size_t i = 0;

    while (length >= 2) {
        unsigned int value = getAlphanumericCode(data[i]) * 45 +
                             getAlphanumericCode(data[i + 1]);
        appendBits(codewords, index, value, 11);
        i += 2;
        length -= 2;
    }

    if (length == 1) {
        unsigned int value = getAlphanumericCode(data[i]);
        appendBits(codewords, index, value, 6);
    }
}

static void appendByte(uint8_t *codewords, size_t *index, const uint8_t *data,
                       size_t length) {
    for (size_t i = 0; i < length; i++) {
        appendBits(codewords, index, data[i], 8);
    }
}

static void appendKanji(uint8_t *codewords, size_t *index, const uint8_t *data,
                        size_t length) {
    for (size_t i = 0; i < length; i += 2) {
        unsigned int value = (data[i] << 8) | data[i + 1];
        value = (value - 0x8140) & 0x3FFF;
        value = (value >> 8) * 0xC0 + (value & 0xFF);
        appendBits(codewords, index, value, 13);
    }
}

static void appendTerminator(uint8_t *codewords, size_t *index,
                             size_t numCodewords) {
    size_t numBits = numCodewords * 8 - *index;
    appendBits(codewords, index, 0, numBits < 4 ? numBits : 4);
}

static void appendPadding(uint8_t *codewords, size_t *index,
                          size_t numCodewords) {
    size_t numBits = ~*index + 1 & 7;
    appendBits(codewords, index, 0, numBits);

    size_t numPaddingCodewords = numCodewords - *index / 8;
    unsigned int value = 0xEC;

    for (size_t i = 0; i < numPaddingCodewords; i++) {
        appendBits(codewords, index, value, 8);
        value ^= 0xFD;
    }
}

/**
 * Encode the data codewords.
 *
 * @param codewords The data codewords
 * @param numCodewords The number of data codewords
 * @param data The data
 * @param segments The segments of the data
 * @param versionClass The version class
 */
void encodeDataCodewords(uint8_t *codewords, size_t numCodewords,
                         const uint8_t *data, const Segment *segments,
                         VersionClass versionClass) {
    memset(codewords, 0, numCodewords);

    size_t index = 0;

    for (const Segment *segment = segments; segment != NULL;
         segment = segment->next) {

        appendModeIndicator(codewords, &index, segment->mode);
        appendCharCountIndicator(
            codewords, &index,
            segment->mode == MODE_KANJI ? segment->length / 2 : segment->length,
            versionClass, segment->mode);

        switch (segment->mode) {
        case MODE_NUMERIC:
            appendNumeric(codewords, &index, data, segment->length);
            break;

        case MODE_ALPHANUMERIC:
            appendAlphanumeric(codewords, &index, data, segment->length);
            break;

        case MODE_BYTE:
            appendByte(codewords, &index, data, segment->length);
            break;

        case MODE_KANJI:
            appendKanji(codewords, &index, data, segment->length);
            break;

        default:
            break;
        }

        data += segment->length;
    }

    appendTerminator(codewords, &index, numCodewords);
    appendPadding(codewords, &index, numCodewords);
}
