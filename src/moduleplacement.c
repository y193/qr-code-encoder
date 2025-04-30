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

#include "moduleplacement.h"
#include "module.h"
#include <string.h>

// {number of coordinates, coordinates 1, coordinates 2, ...}
static const size_t alignmentPatternCoordinates[][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},          {2, 6, 18, 0, 0, 0, 0, 0},
    {2, 6, 22, 0, 0, 0, 0, 0},         {2, 6, 26, 0, 0, 0, 0, 0},
    {2, 6, 30, 0, 0, 0, 0, 0},         {2, 6, 34, 0, 0, 0, 0, 0},
    {3, 6, 22, 38, 0, 0, 0, 0},        {3, 6, 24, 42, 0, 0, 0, 0},
    {3, 6, 26, 46, 0, 0, 0, 0},        {3, 6, 28, 50, 0, 0, 0, 0},
    {3, 6, 30, 54, 0, 0, 0, 0},        {3, 6, 32, 58, 0, 0, 0, 0},
    {3, 6, 34, 62, 0, 0, 0, 0},        {4, 6, 26, 46, 66, 0, 0, 0},
    {4, 6, 26, 48, 70, 0, 0, 0},       {4, 6, 26, 50, 74, 0, 0, 0},
    {4, 6, 30, 54, 78, 0, 0, 0},       {4, 6, 30, 56, 82, 0, 0, 0},
    {4, 6, 30, 58, 86, 0, 0, 0},       {4, 6, 34, 62, 90, 0, 0, 0},
    {5, 6, 28, 50, 72, 94, 0, 0},      {5, 6, 26, 50, 74, 98, 0, 0},
    {5, 6, 30, 54, 78, 102, 0, 0},     {5, 6, 28, 54, 80, 106, 0, 0},
    {5, 6, 32, 58, 84, 110, 0, 0},     {5, 6, 30, 58, 86, 114, 0, 0},
    {5, 6, 34, 62, 90, 118, 0, 0},     {6, 6, 26, 50, 74, 98, 122, 0},
    {6, 6, 30, 54, 78, 102, 126, 0},   {6, 6, 26, 52, 78, 104, 130, 0},
    {6, 6, 30, 56, 82, 108, 134, 0},   {6, 6, 34, 60, 86, 112, 138, 0},
    {6, 6, 30, 58, 86, 114, 142, 0},   {6, 6, 34, 62, 90, 118, 146, 0},
    {7, 6, 30, 54, 78, 102, 126, 150}, {7, 6, 24, 50, 76, 102, 128, 154},
    {7, 6, 28, 54, 80, 106, 132, 158}, {7, 6, 32, 58, 84, 110, 136, 162},
    {7, 6, 26, 54, 82, 110, 138, 166}, {7, 6, 30, 58, 86, 114, 142, 170}};

/**
 * Returns the size of the symbol in number of modules for the given version.
 *
 * @param version The version number
 * @return The size of the symbol in number of modules
 */
size_t getSymbolSizeInNumModules(unsigned int version) {
    return 17 + 4 * version;
}

static void placeHorizontalLine(uint8_t *matrix, size_t size, size_t y,
                                size_t x, size_t length, uint8_t module) {
    for (size_t i = 0; i < length; i++) {
        placeModule(matrix, size, y, x + i, module);
    }
}

static void placeVerticalLine(uint8_t *matrix, size_t size, size_t y, size_t x,
                              size_t length, uint8_t module) {
    for (size_t i = 0; i < length; i++) {
        placeModule(matrix, size, y + i, x, module);
    }
}

static void placeRectangle(uint8_t *matrix, size_t size, size_t y, size_t x,
                           size_t height, size_t width, uint8_t module) {
    placeHorizontalLine(matrix, size, y, x, width - 1, module);
    placeVerticalLine(matrix, size, y, x + width - 1, height - 1, module);
    placeHorizontalLine(matrix, size, y + height - 1, x + 1, width - 1, module);
    placeVerticalLine(matrix, size, y + 1, x, height - 1, module);
}

static void placeFilledRectangle(uint8_t *matrix, size_t size, size_t y,
                                 size_t x, size_t height, size_t width,
                                 uint8_t module) {
    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            placeModule(matrix, size, y + i, x + j, module);
        }
    }
}

static void placeFinderPattern(uint8_t *matrix, size_t size, size_t y,
                               size_t x) {
    uint8_t module = MODULE_FUNCTION | MODULE_DARK;

    placeFilledRectangle(matrix, size, y, x, 7, 7, module);
    placeRectangle(matrix, size, y + 1, x + 1, 5, 5, module ^ MODULE_DARK);
}

static void placeFinderPatterns(uint8_t *matrix, size_t size) {
    placeFinderPattern(matrix, size, 0, 0);
    placeFinderPattern(matrix, size, 0, size - 7);
    placeFinderPattern(matrix, size, size - 7, 0);
}

static void placeSeparators(uint8_t *matrix, size_t size) {
    uint8_t module = MODULE_FUNCTION | MODULE_LIGHT;

    placeVerticalLine(matrix, size, 0, 7, 7, module);
    placeHorizontalLine(matrix, size, 7, 0, 8, module);
    placeVerticalLine(matrix, size, 0, size - 8, 7, module);
    placeHorizontalLine(matrix, size, 7, size - 8, 8, module);
    placeHorizontalLine(matrix, size, size - 8, 0, 8, module);
    placeVerticalLine(matrix, size, size - 7, 7, 7, module);
}

static void placeTimingPatterns(uint8_t *matrix, size_t size) {
    uint8_t module = MODULE_FUNCTION | MODULE_DARK;

    for (size_t i = 8; i < size - 8; i++) {
        placeModule(matrix, size, 6, i, module);
        placeModule(matrix, size, i, 6, module);

        module ^= MODULE_DARK;
    }
}

static void placeAlignmentPattern(uint8_t *matrix, size_t size, size_t y,
                                  size_t x) {
    uint8_t module = MODULE_FUNCTION | MODULE_DARK;

    placeFilledRectangle(matrix, size, y, x, 5, 5, module);
    placeRectangle(matrix, size, y + 1, x + 1, 3, 3, module ^ MODULE_DARK);
}

static void placeAlignmentPatterns(uint8_t *matrix, size_t size,
                                   unsigned int version) {
    const size_t *coordinates = alignmentPatternCoordinates[version - 1];
    size_t numCoordinates = coordinates[0];

    for (size_t i = 1; i <= numCoordinates; i++) {
        for (size_t j = 1; j <= numCoordinates; j++) {
            if ((1 < i && i < numCoordinates) ||
                (1 < j && j < numCoordinates) ||
                (i == numCoordinates && j == numCoordinates)) {
                placeAlignmentPattern(matrix, size, coordinates[i] - 2,
                                      coordinates[j] - 2);
            }
        }
    }
}

static void placeDarkModule(uint8_t *matrix, size_t size,
                            unsigned int version) {
    uint8_t module = MODULE_FUNCTION | MODULE_DARK;

    placeModule(matrix, size, 4 * version + 9, 8, module);
}

static void reserveFormatInformation(uint8_t *matrix, size_t size) {
    uint8_t module = MODULE_FUNCTION | MODULE_BLANK;

    placeVerticalLine(matrix, size, 0, 8, 9, module);
    placeHorizontalLine(matrix, size, 8, 0, 9, module);
    placeHorizontalLine(matrix, size, 8, size - 8, 8, module);
    placeVerticalLine(matrix, size, size - 8, 8, 8, module);
}

static void reserveVersionInformation(uint8_t *matrix, size_t size,
                                      unsigned int version) {
    if (version < 7) {
        return;
    }

    uint8_t module = MODULE_FUNCTION | MODULE_BLANK;

    placeFilledRectangle(matrix, size, 0, size - 11, 6, 3, module);
    placeFilledRectangle(matrix, size, size - 11, 0, 3, 6, module);
}

static void placeFunctionPatterns(uint8_t *matrix, size_t size,
                                  unsigned int version) {
    reserveFormatInformation(matrix, size);
    reserveVersionInformation(matrix, size, version);
    placeFinderPatterns(matrix, size);
    placeSeparators(matrix, size);
    placeTimingPatterns(matrix, size);
    placeAlignmentPatterns(matrix, size, version);
    placeDarkModule(matrix, size, version);
}

static uint8_t getBit(const uint8_t *array, size_t index) {
    return array[index / 8] >> (7 - index % 8) & 1;
}

static void placeCodewordModules(uint8_t *matrix, size_t size,
                                 const uint8_t *codewords) {
    size_t x = size - 1;
    size_t y = size - 1;
    size_t vy = -1;
    size_t index = 0;

    for (size_t i = 0; i < size / 2; i++) {
        for (size_t j = 0; j < size; j++) {
            for (size_t k = 0; k < 2; k++) {
                if (getModule(matrix, size, y + vy * j, x - k) == 0) {
                    placeModule(matrix, size, y + vy * j, x - k,
                                getBit(codewords, index++));
                }
            }
        }

        x -= x == 8 ? 3 : 2;
        y ^= size - 1;
        vy = -vy;
    }
}

/**
 * Places the modules in the matrix.
 *
 * @param matrix The matrix
 * @param size The size of the symbol in number of modules
 * @param version The version number
 * @param codewords The codewords
 */
void placeModules(uint8_t *matrix, size_t size, unsigned int version,
                  const uint8_t *codewords) {
    memset(matrix, 0, size * size);

    placeFunctionPatterns(matrix, size, version);
    placeCodewordModules(matrix, size, codewords);
}
