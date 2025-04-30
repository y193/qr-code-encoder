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

#ifndef DATAMASKING_H
#define DATAMASKING_H

#include <stddef.h>
#include <stdint.h>

extern void applyDataMaskPattern(uint8_t *masked, const uint8_t *unmasked,
                                 size_t size, unsigned int dataMaskPattern);
extern unsigned int calculatePenaltyScoreCondition1(const uint8_t *masked,
                                                    size_t size);
extern unsigned int calculatePenaltyScoreCondition2(const uint8_t *masked,
                                                    size_t size);
extern unsigned int calculatePenaltyScoreCondition3(const uint8_t *masked,
                                                    size_t size);
extern unsigned int calculatePenaltyScoreCondition4(const uint8_t *masked,
                                                    size_t size);
extern unsigned int
applyDataMaskPatternLowestPenaltyScore(uint8_t *masked, const uint8_t *unmasked,
                                       size_t size);

#endif /* DATAMASKING_H */
