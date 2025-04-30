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

#include "formatandversion.h"
#include "module.h"

static uint16_t calculateBCHBits(size_t numTotalBits, size_t numDataBits,
                                 uint8_t dataBits, uint16_t generatorPoly) {
    uint32_t bchBits = dataBits << (numTotalBits - numDataBits);

    for (size_t i = 0; i < numDataBits; i++) {
        if (bchBits & (1 << (numTotalBits - 1 - i))) {
            bchBits ^= generatorPoly << (numDataBits - 1 - i);
        }
    }

    return bchBits;
}

/**
 * Place format information in the QR code matrix.
 *
 * @param matrix The QR code matrix
 * @param size The size of the symbol in number of modules
 * @param ecLevel The error correction level
 * @param dataMaskPattern The data mask pattern
 */
void placeFormatInformation(uint8_t *matrix, size_t size,
                            ErrorCorrectionLevel ecLevel,
                            unsigned int dataMaskPattern) {
    uint8_t ecLevelBits = 5 - ecLevel & 3;
    uint8_t dataMaskPatternBits = dataMaskPattern & 7;

    uint8_t dataBits = (ecLevelBits << 3) | dataMaskPatternBits;
    uint16_t bchBits = calculateBCHBits(15, 5, dataBits, 0x537);
    uint16_t formatInformation = (dataBits << 10) | bchBits;
    formatInformation ^= 0x5412;

    size_t y = 0;
    size_t x = size - 1;

    for (size_t i = 0; i < 15; i++) {
        if (i == 6) {
            y = 7;
        } else if (i == 8) {
            y = size - 7;
            x = 7;
        } else if (i == 9) {
            x = 5;
        }

        uint8_t module = (formatInformation >> i) & 1;

        placeModule(matrix, size, y++, 8, module);
        placeModule(matrix, size, 8, x--, module);
    }
}

/**
 * Place version information in the QR code matrix.
 *
 * @param matrix The QR code matrix
 * @param size The size of the symbol in number of modules
 * @param version The version number
 */
void placeVersionInformation(uint8_t *matrix, size_t size,
                             unsigned int version) {
    if (version < 7) {
        return;
    }

    uint8_t dataBits = version & 63;
    uint16_t bchBits = calculateBCHBits(18, 6, dataBits, 0x1F25);
    uint32_t versionInformation = (dataBits << 12) | bchBits;

    for (size_t i = 0; i < 6; i++) {
        for (size_t j = 0; j < 3; j++) {
            uint8_t module = (versionInformation >> (3 * i + j)) & 1;

            placeModule(matrix, size, i, size - 11 + j, module);
            placeModule(matrix, size, size - 11 + j, i, module);
        }
    }
}
