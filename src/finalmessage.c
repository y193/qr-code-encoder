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

#include "finalmessage.h"

static void interleaveCodewords(uint8_t *interleaved, const uint8_t *codewords,
                                RSBlock block) {
    size_t index = 0;

    for (size_t i = 0; i < block.numDataCodewords1; i++) {
        for (size_t j = 0; j < block.numBlocks1; j++) {
            interleaved[index++] = codewords[i + j * block.numDataCodewords1];
        }

        for (size_t j = 0; j < block.numBlocks2; j++) {
            interleaved[index++] =
                codewords[i + j * block.numDataCodewords2 +
                          block.numBlocks1 * block.numDataCodewords1];
        }
    }

    for (size_t i = block.numDataCodewords1; i < block.numDataCodewords2; i++) {
        for (size_t j = 0; j < block.numBlocks2; j++) {
            interleaved[index++] =
                codewords[i + j * block.numDataCodewords2 +
                          block.numBlocks1 * block.numDataCodewords1];
        }
    }
}

/**
 * Constructs the final message by interleaving the data and error correction
 * codewords.
 *
 * @param finalMessage The final message
 * @param dataCodewords The data codewords
 * @param ecCodewords The error correction codewords
 * @param block The RS block
 */
void constructFinalMessage(uint8_t *finalMessage, const uint8_t *dataCodewords,
                           const uint8_t *ecCodewords, RSBlock block) {
    size_t numDataCodewords = block.numBlocks1 * block.numDataCodewords1 +
                              block.numBlocks2 * block.numDataCodewords2;
    size_t numECCodewords = block.numBlocks1 * block.numECCodewords +
                            block.numBlocks2 * block.numECCodewords;

    interleaveCodewords(finalMessage, dataCodewords, block);
    interleaveCodewords(finalMessage + numDataCodewords, ecCodewords,
                        (RSBlock){block.numBlocks1, block.numECCodewords,
                                  block.numBlocks2, block.numECCodewords, 0});

    // remainder bits
    finalMessage[numDataCodewords + numECCodewords] = 0;
}
