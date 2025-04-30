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
#include "dataencoding.h"
#include "datamasking.h"
#include "errorcorrection.h"
#include "finalmessage.h"
#include "formatandversion.h"
#include "module.h"
#include "moduleplacement.h"
#include "rsblock.h"
#include "segment.h"
#include "typedefs.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_DATA_LENGTH 7089

#define printUsageAndExit()                                                    \
    do {                                                                       \
        fprintf(stderr, "Usage: qrce.exe "                                     \
                        "[/E ErrorCorrectionLevel] [/V Version] [/K] [/O]\n\n" \
                        "Options:\n"                                           \
                        "  /E ErrorCorrectionLevel   "                         \
                        "Error correction level. L, M, Q, or H.\n"             \
                        "  /V Version                "                         \
                        "Size of the symbol represented by version 1 to 40.\n" \
                        "  /K                        "                         \
                        "Use Kanji mode.\n"                                    \
                        "  /O                        "                         \
                        "Optimize the length of the bit string.\n");           \
        return EXIT_FAILURE;                                                   \
    } while (0)

static int parseErrorCorrectionLevel(const char *v) {
    if (v[1] != '\0') {
        return -1;
    }

    switch (v[0]) {
    case 'L':
    case 'l':
        return ERROR_CORRECTION_LEVEL_L;

    case 'M':
    case 'm':
        return ERROR_CORRECTION_LEVEL_M;

    case 'Q':
    case 'q':
        return ERROR_CORRECTION_LEVEL_Q;

    case 'H':
    case 'h':
        return ERROR_CORRECTION_LEVEL_H;

    default:
        return -1;
    }
}

static int parseVersion(const char *v) {
    char *endptr;
    long version = strtol(v, &endptr, 10);

    return *endptr == '\0' && 1 <= version && version <= 40 ? (int)version : -1;
}

int main(int argc, char *const *argv) {
    int ecLevel = ERROR_CORRECTION_LEVEL_L;
    int version = -1;
    bool useKanjiMode = false;
    bool useOptimization = false;

    int option = 0;

    for (int i = 1; i < argc; i++) {
        char *v = argv[i];

        switch (option) {
        case 0:
            if (v[0] != '/' || v[1] == '\0' || v[2] != '\0') {
                printUsageAndExit();
            }

            switch (v[1]) {
            case 'K':
            case 'k':
                useKanjiMode = true;
                continue;

            case 'O':
            case 'o':
                useOptimization = true;
                continue;

            default:
                option = v[1];
                continue;
            }

        case 'E':
        case 'e':
            if ((ecLevel = parseErrorCorrectionLevel(v)) == -1) {
                printUsageAndExit();
            }
            break;

        case 'V':
        case 'v':
            if ((version = parseVersion(v)) == -1) {
                printUsageAndExit();
            }
            break;

        default:
            printUsageAndExit();
        }

        option = 0;
    }

    if (option != 0) {
        printUsageAndExit();
    }

    uint8_t *data = malloc(sizeof(uint8_t) * (MAX_DATA_LENGTH + 1));

    if (data == NULL) {
        fprintf(stderr, "Out of memory\n");
        return EXIT_FAILURE;
    }

    int length = fread(data, sizeof(uint8_t), MAX_DATA_LENGTH + 1, stdin);

    if (ferror(stdin)) {
        perror("Read error");
        return EXIT_FAILURE;
    }

    if (length > MAX_DATA_LENGTH) {
        fprintf(stderr, "Input is too long\n");
        return EXIT_FAILURE;
    }

    Segment *segments;
    int recommendedVersion;
    VersionClass versionClass;

    if (useOptimization) {
        for (versionClass = VERSION_CLASS_SMALL;
             versionClass <= VERSION_CLASS_LARGE; versionClass++) {

            segments = createMixedModeSegments(data, length, useKanjiMode,
                                               versionClass);

            if (segments == NULL) {
                fprintf(stderr, "Out of memory\n");
                return EXIT_FAILURE;
            }

            recommendedVersion =
                recommendVersion(segments, ecLevel, versionClass);

            if (recommendedVersion != -1) {
                break;
            }

            freeSegments(segments);
        }

    } else {
        segments = createModeSegment(data, length, useKanjiMode);

        if (segments == NULL) {
            fprintf(stderr, "Out of memory\n");
            return EXIT_FAILURE;
        }

        for (versionClass = VERSION_CLASS_SMALL;
             versionClass <= VERSION_CLASS_LARGE; versionClass++) {

            recommendedVersion =
                recommendVersion(segments, ecLevel, versionClass);

            if (recommendedVersion != -1) {
                break;
            }
        }
    }

    if (recommendedVersion == -1) {
        fprintf(stderr, "Input is too long\n");
        return EXIT_FAILURE;
    }

    if (version == -1) {
        version = recommendedVersion;

    } else if (version < recommendedVersion) {
        fprintf(stderr, "Input is too long for version %d\n", version);
        return EXIT_FAILURE;
    }

    RSBlock rsBlock = getRSBlock(version, ecLevel);

    size_t numDataCodewords = rsBlock.numBlocks1 * rsBlock.numDataCodewords1 +
                              rsBlock.numBlocks2 * rsBlock.numDataCodewords2;
    size_t numECCodewords =
        (rsBlock.numBlocks1 + rsBlock.numBlocks2) * rsBlock.numECCodewords;

    size_t numCodewords = numDataCodewords + numECCodewords;
    uint8_t *codewords = malloc((2 * numCodewords + 1) * sizeof(uint8_t));

    if (codewords == NULL) {
        fprintf(stderr, "Out of memory\n");
        return EXIT_FAILURE;
    }

    size_t symbolSize = getSymbolSizeInNumModules(version);
    uint8_t *matrix = malloc(2 * symbolSize * symbolSize * sizeof(uint8_t));

    if (matrix == NULL) {
        fprintf(stderr, "Out of memory\n");
        return EXIT_FAILURE;
    }

    uint8_t *dataCodewords = codewords;
    uint8_t *ecCodewords = dataCodewords + numDataCodewords;
    uint8_t *finalMessage = ecCodewords + numECCodewords;
    uint8_t *unmasked = matrix;
    uint8_t *masked = unmasked + symbolSize * symbolSize;

    encodeDataCodewords(dataCodewords, numDataCodewords, data, segments,
                        versionClass);

    freeSegments(segments);
    free(data);

    encodeErrorCorrectionCodewords(ecCodewords, dataCodewords, rsBlock);
    constructFinalMessage(finalMessage, dataCodewords, ecCodewords, rsBlock);
    placeModules(matrix, symbolSize, version, finalMessage);

    free(codewords);

    unsigned int dataMaskPattern =
        applyDataMaskPatternLowestPenaltyScore(masked, unmasked, symbolSize);

    placeFormatInformation(masked, symbolSize, ecLevel, dataMaskPattern);
    placeVersionInformation(masked, symbolSize, version);

    printf("%d ", version);

    for (size_t y = 0; y < symbolSize; y++) {
        for (size_t x = 0; x < symbolSize; x++) {
            uint8_t module = getModule(masked, symbolSize, y, x);
            putchar(module + '0');
        }
    }

    free(matrix);

    return EXIT_SUCCESS;
}
