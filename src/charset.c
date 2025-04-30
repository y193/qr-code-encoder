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

#include "charset.h"

static const int alphanumericTable[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    36, -1, -1, -1, 37, 38, -1, -1, -1, -1, 39, 40, -1, 41, 42, 43,
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  44, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, -1};

/**
 * Get the alphanumeric code of the byte.
 *
 * @param b The byte
 * @return The alphanumeric code
 */
int getAlphanumericCode(uint8_t b) {
    return b <= 'Z' ? alphanumericTable[b] : -1;
}

/**
 * Check if the byte is in the numeric subset.
 *
 * @param b The byte
 * @return true if the byte is in the numeric subset
 */
bool isNumeric(uint8_t b) {
    return '0' <= b && b <= '9';
}

/**
 * Check if the byte is in the alphanumeric subset.
 *
 * @param b The byte
 * @return true if the byte is in the alphanumeric subset
 */
bool isAlphanumeric(uint8_t b) {
    return getAlphanumericCode(b) != -1;
}

/**
 * Check if the bytes are in the Shift_JIS Kanji subset.
 *
 * @param b1 The first byte
 * @param b2 The second byte
 * @return true if the bytes are in the Shift_JIS Kanji subset
 */
bool isShiftJISKanji(uint8_t b1, uint8_t b2) {
    return 0x81 <= b1 && (b1 <= 0x9F || 0xE0 <= b1) && b1 <= 0xEB &&
           0x40 <= b2 && b2 != 0x7F && b2 <= 0xFC && (b1 < 0xEB || b2 <= 0xBF);
}

/**
 * Check if the byte is in the exclusive alphanumeric subset.
 *
 * @param b The byte
 * @return true if the byte is in the exclusive alphanumeric subset
 */
bool isExclusiveAlphanumericSubset(uint8_t b) {
    return isAlphanumeric(b) && !isNumeric(b);
}

/**
 * Check if the byte is in the exclusive 8-bit byte subset.
 *
 * @param b The byte
 * @return true if the byte is in the exclusive 8-bit byte subset
 */
bool isExclusive8BitByteSubset(uint8_t b) {
    return (b < 0x80 && !isAlphanumeric(b)) || (0x9F < b && b < 0xE0);
}
