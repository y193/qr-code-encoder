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

#include "dataanalysis.h"
#include "charset.h"

static Mode selectMode(const uint8_t *data, size_t length, bool useKanjiMode) {
    size_t i = 0;

    while (i < length && isNumeric(data[i])) {
        i++;
    }

    if (i >= length) {
        return MODE_NUMERIC;
    }

    while (i < length && isAlphanumeric(data[i])) {
        i++;
    }

    if (i >= length) {
        return MODE_ALPHANUMERIC;
    }

    if (useKanjiMode && i == 0 && length % 2 == 0) {
        while (i < length && isShiftJISKanji(data[i], data[i + 1])) {
            i += 2;
        }

        if (i >= length) {
            return MODE_KANJI;
        }
    }

    return MODE_BYTE;
}

/**
 * Create the segment of the data.
 *
 * @param data The data
 * @param length The length of the data
 * @param useKanjiMode Whether to use Kanji mode
 * @return The segment of the data
 */
Segment *createModeSegment(const uint8_t *data, size_t length,
                           bool useKanjiMode) {
    Mode mode = selectMode(data, length, useKanjiMode);

    return newSegment(mode, length);
}

static Mode selectInitialMode(const uint8_t *data, bool useKanjiMode,
                              VersionClass versionClass) {
    if (isNumeric(data[0])) {
        size_t lookahead = (const size_t[]){4, 4, 5}[versionClass];

        for (size_t i = 1; i < lookahead; i++) {
            if (isNumeric(data[i])) {
                continue;
            } else if (isExclusive8BitByteSubset(data[i])) {
                return MODE_BYTE;
            } else {
                break;
            }
        }

        lookahead = (const size_t[]){7, 8, 9}[versionClass];

        for (size_t i = 1; i < lookahead; i++) {
            if (isNumeric(data[i])) {
                continue;
            } else if (isAlphanumeric(data[i])) {
                return MODE_ALPHANUMERIC;
            } else {
                break;
            }
        }

        return MODE_NUMERIC;
    }

    if (isAlphanumeric(data[0])) {
        size_t lookahead = (const size_t[]){6, 7, 8}[versionClass];

        for (size_t i = 1; i < lookahead; i++) {
            if (!isAlphanumeric(data[i])) {
                return MODE_BYTE;
            }
        }

        return MODE_ALPHANUMERIC;
    }

    if (useKanjiMode && isShiftJISKanji(data[0], data[1])) {
        if (!isExclusive8BitByteSubset(data[2])) {
            return MODE_KANJI;
        }

        size_t lookahead = (const size_t[]){5, 5, 6}[versionClass] * 2;

        for (size_t i = 0; i < lookahead; i += 2) {
            if (!isShiftJISKanji(data[3 + i], data[4 + i])) {
                return MODE_KANJI;
            }
        }
    }

    return MODE_BYTE;
}

static Mode recommendNextMode(const uint8_t *data, size_t length,
                              bool useKanjiMode) {
    if (isNumeric(data[0])) {
        return MODE_NUMERIC;
    }

    if (isAlphanumeric(data[0])) {
        return MODE_ALPHANUMERIC;
    }

    if (useKanjiMode && length >= 2) {
        if (isShiftJISKanji(data[0], data[1])) {
            return MODE_KANJI;
        }
    }

    return MODE_BYTE;
}

/**
 * Create the segments of the data. Minimize the bit stream length using the
 * algorithm in Annex J of JIS X 0510:2018.
 *
 * @param data The data
 * @param length The length of the data
 * @param useKanjiMode Whether to use Kanji mode
 * @param versionClass The version class
 * @return The segments of the data
 */
Segment *createMixedModeSegments(const uint8_t *data, size_t length,
                                 bool useKanjiMode, VersionClass versionClass) {
    if (length < 9 || (useKanjiMode && length < 15)) {
        return createModeSegment(data, length, useKanjiMode);
    }

    Segment *segments = NULL;

    size_t byteToKanjiRunLength = (const size_t[]){9, 12, 13}[versionClass] * 2;
    size_t byteToAlnumRunLength = (const size_t[]){11, 15, 16}[versionClass];
    size_t byteToNum1RunLength = (const size_t[]){6, 7, 8}[versionClass];
    size_t byteToNum2RunLength = (const size_t[]){6, 8, 9}[versionClass];
    size_t alnumToNumRunLength = (const size_t[]){13, 15, 17}[versionClass];

    size_t kanjiRunLength = 0;
    size_t alnumRunLength = 0;
    size_t numRunLength = 0;

    Mode mode = selectInitialMode(data, useKanjiMode, versionClass);
    size_t chrlen = mode == MODE_KANJI ? 2 : 1;

    Mode segmentMode = mode;
    size_t segmentLength = chrlen;

    for (size_t i = chrlen; i < length; i += chrlen) {
        mode = recommendNextMode(data + i, length - i, useKanjiMode);
        chrlen = mode == MODE_KANJI ? 2 : 1;

        if (segmentMode == MODE_BYTE && mode == MODE_KANJI) {
            segmentLength += alnumRunLength + numRunLength;

            kanjiRunLength += chrlen;
            alnumRunLength = 0;
            numRunLength = 0;

            if (kanjiRunLength < byteToKanjiRunLength) {
                continue;
            }

        } else if (segmentMode == MODE_BYTE && mode == MODE_ALPHANUMERIC) {
            segmentLength += kanjiRunLength + numRunLength;

            kanjiRunLength = 0;
            alnumRunLength += chrlen;
            numRunLength = 0;

            if (alnumRunLength < byteToAlnumRunLength) {
                continue;
            }

        } else if (segmentMode == MODE_BYTE && mode == MODE_NUMERIC) {
            segmentLength += kanjiRunLength + alnumRunLength;

            kanjiRunLength = 0;
            alnumRunLength = 0;
            numRunLength += chrlen;

            if (numRunLength < byteToNum1RunLength) {
                continue;
            }

            if (numRunLength < byteToNum2RunLength) {
                if (i < length - 1 &&
                    !isExclusiveAlphanumericSubset(data[i + 1])) {
                    continue;
                }
            }

        } else if (segmentMode == MODE_ALPHANUMERIC && mode == MODE_NUMERIC) {
            numRunLength += chrlen;

            if (numRunLength < alnumToNumRunLength) {
                continue;
            }

        } else {
            segmentLength += kanjiRunLength + alnumRunLength + numRunLength;

            kanjiRunLength = 0;
            alnumRunLength = 0;
            numRunLength = 0;

            if (segmentMode == mode) {
                segmentLength += chrlen;
                continue;
            }
        }

        Segment *segment = newSegment(segmentMode, segmentLength);

        if (segment == NULL) {
            return NULL;
        }

        segments = addSegment(segments, segment);

        segmentMode = mode;
        segmentLength = kanjiRunLength + alnumRunLength + numRunLength;

        if (segmentLength == 0) {
            segmentLength = chrlen;
        }

        kanjiRunLength = 0;
        alnumRunLength = 0;
        numRunLength = 0;
    }

    segmentLength += kanjiRunLength + alnumRunLength + numRunLength;

    Segment *segment = newSegment(segmentMode, segmentLength);

    if (segment == NULL) {
        return NULL;
    }

    segments = addSegment(segments, segment);

    return segments;
}
