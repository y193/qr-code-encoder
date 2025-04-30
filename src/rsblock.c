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

#include "rsblock.h"

// RS block table {numBlocks1, numDataCodewords1, numBlocks2, numECCodewords}
static const size_t rsBlockTable[][4][4] = {
    {{1, 19, 0, 7}, {1, 16, 0, 10}, {1, 13, 0, 13}, {1, 9, 0, 17}},
    {{1, 34, 0, 10}, {1, 28, 0, 16}, {1, 22, 0, 22}, {1, 16, 0, 28}},
    {{1, 55, 0, 15}, {1, 44, 0, 26}, {2, 17, 0, 18}, {2, 13, 0, 22}},
    {{1, 80, 0, 20}, {2, 32, 0, 18}, {2, 24, 0, 26}, {4, 9, 0, 16}},
    {{1, 108, 0, 26}, {2, 43, 0, 24}, {2, 15, 2, 18}, {2, 11, 2, 22}},
    {{2, 68, 0, 18}, {4, 27, 0, 16}, {4, 19, 0, 24}, {4, 15, 0, 28}},
    {{2, 78, 0, 20}, {4, 31, 0, 18}, {2, 14, 4, 18}, {4, 13, 1, 26}},
    {{2, 97, 0, 24}, {2, 38, 2, 22}, {4, 18, 2, 22}, {4, 14, 2, 26}},
    {{2, 116, 0, 30}, {3, 36, 2, 22}, {4, 16, 4, 20}, {4, 12, 4, 24}},
    {{2, 68, 2, 18}, {4, 43, 1, 26}, {6, 19, 2, 24}, {6, 15, 2, 28}},
    {{4, 81, 0, 20}, {1, 50, 4, 30}, {4, 22, 4, 28}, {3, 12, 8, 24}},
    {{2, 92, 2, 24}, {6, 36, 2, 22}, {4, 20, 6, 26}, {7, 14, 4, 28}},
    {{4, 107, 0, 26}, {8, 37, 1, 22}, {8, 20, 4, 24}, {12, 11, 4, 22}},
    {{3, 115, 1, 30}, {4, 40, 5, 24}, {11, 16, 5, 20}, {11, 12, 5, 24}},
    {{5, 87, 1, 22}, {5, 41, 5, 24}, {5, 24, 7, 30}, {11, 12, 7, 24}},
    {{5, 98, 1, 24}, {7, 45, 3, 28}, {15, 19, 2, 24}, {3, 15, 13, 30}},
    {{1, 107, 5, 28}, {10, 46, 1, 28}, {1, 22, 15, 28}, {2, 14, 17, 28}},
    {{5, 120, 1, 30}, {9, 43, 4, 26}, {17, 22, 1, 28}, {2, 14, 19, 28}},
    {{3, 113, 4, 28}, {3, 44, 11, 26}, {17, 21, 4, 26}, {9, 13, 16, 26}},
    {{3, 107, 5, 28}, {3, 41, 13, 26}, {15, 24, 5, 30}, {15, 15, 10, 28}},
    {{4, 116, 4, 28}, {17, 42, 0, 26}, {17, 22, 6, 28}, {19, 16, 6, 30}},
    {{2, 111, 7, 28}, {17, 46, 0, 28}, {7, 24, 16, 30}, {34, 13, 0, 24}},
    {{4, 121, 5, 30}, {4, 47, 14, 28}, {11, 24, 14, 30}, {16, 15, 14, 30}},
    {{6, 117, 4, 30}, {6, 45, 14, 28}, {11, 24, 16, 30}, {30, 16, 2, 30}},
    {{8, 106, 4, 26}, {8, 47, 13, 28}, {7, 24, 22, 30}, {22, 15, 13, 30}},
    {{10, 114, 2, 28}, {19, 46, 4, 28}, {28, 22, 6, 28}, {33, 16, 4, 30}},
    {{8, 122, 4, 30}, {22, 45, 3, 28}, {8, 23, 26, 30}, {12, 15, 28, 30}},
    {{3, 117, 10, 30}, {3, 45, 23, 28}, {4, 24, 31, 30}, {11, 15, 31, 30}},
    {{7, 116, 7, 30}, {21, 45, 7, 28}, {1, 23, 37, 30}, {19, 15, 26, 30}},
    {{5, 115, 10, 30}, {19, 47, 10, 28}, {15, 24, 25, 30}, {23, 15, 25, 30}},
    {{13, 115, 3, 30}, {2, 46, 29, 28}, {42, 24, 1, 30}, {23, 15, 28, 30}},
    {{17, 115, 0, 30}, {10, 46, 23, 28}, {10, 24, 35, 30}, {19, 15, 35, 30}},
    {{17, 115, 1, 30}, {14, 46, 21, 28}, {29, 24, 19, 30}, {11, 15, 46, 30}},
    {{13, 115, 6, 30}, {14, 46, 23, 28}, {44, 24, 7, 30}, {59, 16, 1, 30}},
    {{12, 121, 7, 30}, {12, 47, 26, 28}, {39, 24, 14, 30}, {22, 15, 41, 30}},
    {{6, 121, 14, 30}, {6, 47, 34, 28}, {46, 24, 10, 30}, {2, 15, 64, 30}},
    {{17, 122, 4, 30}, {29, 46, 14, 28}, {49, 24, 10, 30}, {24, 15, 46, 30}},
    {{4, 122, 18, 30}, {13, 46, 32, 28}, {48, 24, 14, 30}, {42, 15, 32, 30}},
    {{20, 117, 4, 30}, {40, 47, 7, 28}, {43, 24, 22, 30}, {10, 15, 67, 30}},
    {{19, 118, 6, 30}, {18, 47, 31, 28}, {34, 24, 34, 30}, {20, 15, 61, 30}}};

/**
 * Get the RS block for the given version and error correction level.
 *
 * @param version The version number
 * @param ecLevel The error correction level
 * @return The RS block
 */
RSBlock getRSBlock(unsigned int version, ErrorCorrectionLevel ecLevel) {
    const size_t *block = rsBlockTable[version - 1][ecLevel];

    return (RSBlock){block[0], block[1], block[2],
                     block[2] == 0 ? 0 : block[1] + 1, block[3]};
}
