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

#include "datamasking.h"
#include "module.h"
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>

#define N1 3
#define N2 3
#define N3 40
#define N4 10

static bool maskPatternGenerationCondition0(size_t y, size_t x) {
    return (y + x) % 2 == 0;
}

static bool maskPatternGenerationCondition1(size_t y, size_t x) {
    (void)x;
    return y % 2 == 0;
}

static bool maskPatternGenerationCondition2(size_t y, size_t x) {
    (void)y;
    return x % 3 == 0;
}

static bool maskPatternGenerationCondition3(size_t y, size_t x) {
    return (y + x) % 3 == 0;
}

static bool maskPatternGenerationCondition4(size_t y, size_t x) {
    return ((y / 2) + (x / 3)) % 2 == 0;
}

static bool maskPatternGenerationCondition5(size_t y, size_t x) {
    return (y * x) % 2 + (y * x) % 3 == 0;
}

static bool maskPatternGenerationCondition6(size_t y, size_t x) {
    return ((y * x) % 2 + (y * x) % 3) % 2 == 0;
}

static bool maskPatternGenerationCondition7(size_t y, size_t x) {
    return ((y + x) % 2 + (y * x) % 3) % 2 == 0;
}

static bool (*const maskPatternGenerationConditions[])(size_t, size_t) = {
    maskPatternGenerationCondition0, maskPatternGenerationCondition1,
    maskPatternGenerationCondition2, maskPatternGenerationCondition3,
    maskPatternGenerationCondition4, maskPatternGenerationCondition5,
    maskPatternGenerationCondition6, maskPatternGenerationCondition7};

/**
 * Applies the data mask pattern to the unmasked matrix.
 *
 * @param masked The masked matrix
 * @param unmasked The unmasked matrix
 * @param size The size of the symbol in number of modules
 * @param dataMaskPattern The data mask pattern to apply
 */
void applyDataMaskPattern(uint8_t *masked, const uint8_t *unmasked, size_t size,
                          unsigned int dataMaskPattern) {
    bool (*condition)(size_t, size_t);
    condition = maskPatternGenerationConditions[dataMaskPattern];

    for (size_t y = 0; y < size; y++) {
        for (size_t x = 0; x < size; x++) {
            uint8_t module = getModule(unmasked, size, y, x);

            if (module & MODULE_FUNCTION) {
                placeModule(masked, size, y, x, module ^ MODULE_FUNCTION);
            } else {
                placeModule(masked, size, y, x, module ^ condition(y, x));
            }
        }
    }
}

/**
 * Calculates the penalty score for evaluation condition 1.
 *
 * @param masked The masked matrix
 * @param size The size of the symbol in number of modules
 * @return The penalty score for evaluation condition 1
 */
unsigned int calculatePenaltyScoreCondition1(const uint8_t *masked,
                                             size_t size) {
    unsigned int penaltyScore = 0;

    for (size_t h = 0; h < 2; h++) {
        for (size_t i = 0; i < size; i++) {
            uint8_t feature = 0;
            size_t length = 0;

            for (size_t j = 0; j < size; j++) {
                uint8_t module =
                    getModule(masked, size, h * i + !h * j, !h * i + h * j);

                if (module == feature) {
                    length++;
                    continue;
                }

                if (length >= 5 && feature != MODULE_BLANK) {
                    penaltyScore += N1 + (length - 5);
                }

                feature = module;
                length = 1;
            }

            if (length >= 5 && feature != MODULE_BLANK) {
                penaltyScore += N1 + (length - 5);
            }
        }
    }

    return penaltyScore;
}

/**
 * Calculates the penalty score for evaluation condition 2.
 *
 * @param masked The masked matrix
 * @param size The size of the symbol in number of modules
 * @return The penalty score for evaluation condition 2
 */
unsigned int calculatePenaltyScoreCondition2(const uint8_t *masked,
                                             size_t size) {
    unsigned int penaltyScore = 0;

    for (size_t i = 0; i < size - 1; i++) {
        for (size_t j = 0; j < size - 1; j++) {
            uint8_t module = getModule(masked, size, i, j);

            if (module == getModule(masked, size, i, j + 1) &&
                module == getModule(masked, size, i + 1, j) &&
                module == getModule(masked, size, i + 1, j + 1)) {
                penaltyScore += N2;
            }
        }
    }

    return penaltyScore;
}

/**
 * Calculates the penalty score for evaluation condition 3.
 *
 * @param masked The masked matrix
 * @param size The size of the symbol in number of modules
 * @return The penalty score for evaluation condition 3
 */
unsigned int calculatePenaltyScoreCondition3(const uint8_t *masked,
                                             size_t size) {
    unsigned int penaltyScore = 0;

    for (size_t h = 0; h < 2; h++) {
        for (size_t i = 0; i < size; i++) {
            unsigned int feature = 0;
            size_t length = 0;

            for (size_t j = 0; j < size; j++) {
                uint8_t module =
                    getModule(masked, size, h * i + !h * j, !h * i + h * j);

                if (module == MODULE_BLANK) {
                    feature = 0;
                    length = 0;
                    continue;
                }

                feature = ((feature << 1) | module) & 0x7FF;
                length++;

                if (length >= 11 && (feature == 0x5D || feature == 0x5D0)) {
                    penaltyScore += N3;
                }
            }
        }
    }

    return penaltyScore;
}

/**
 * Calculates the penalty score for evaluation condition 4.
 *
 * @param masked The masked matrix
 * @param size The size of the symbol in number of modules
 * @return The penalty score for evaluation condition 4
 */
unsigned int calculatePenaltyScoreCondition4(const uint8_t *masked,
                                             size_t size) {
    int numTotalModules = size * size;
    int numDarkModules = 0;

    for (size_t i = 0; i < size; i++) {
        for (size_t j = 0; j < size; j++) {
            numDarkModules += getModule(masked, size, i, j) == MODULE_DARK;
        }
    }

    return abs(numDarkModules * 2 - numTotalModules) * 10 / numTotalModules *
           N4;
}

static unsigned int calculatePenaltyScore(const uint8_t *masked, size_t size) {
    return calculatePenaltyScoreCondition1(masked, size) +
           calculatePenaltyScoreCondition2(masked, size) +
           calculatePenaltyScoreCondition3(masked, size) +
           calculatePenaltyScoreCondition4(masked, size);
}

/**
 * Applies the data mask pattern to the unmasked matrix and returns the
 * pattern with the lowest penalty score.
 *
 * @param masked The masked matrix
 * @param unmasked The unmasked matrix
 * @param size The size of the symbol in number of modules
 * @return The data mask pattern with the lowest penalty score
 */
unsigned int applyDataMaskPatternLowestPenaltyScore(uint8_t *masked,
                                                    const uint8_t *unmasked,
                                                    size_t size) {
    unsigned int lowestPenaltyScore = UINT_MAX;
    unsigned int dataMaskPattern;

    for (unsigned int pattern = 0; pattern < 8; pattern++) {
        applyDataMaskPattern(masked, unmasked, size, pattern);
        unsigned int penaltyScore = calculatePenaltyScore(masked, size);

        if (lowestPenaltyScore > penaltyScore) {
            lowestPenaltyScore = penaltyScore;
            dataMaskPattern = pattern;
        }
    }

    applyDataMaskPattern(masked, unmasked, size, dataMaskPattern);

    return dataMaskPattern;
}
