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

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

typedef enum {
    ERROR_CORRECTION_LEVEL_L = 0,
    ERROR_CORRECTION_LEVEL_M = 1,
    ERROR_CORRECTION_LEVEL_Q = 2,
    ERROR_CORRECTION_LEVEL_H = 3
} ErrorCorrectionLevel;

typedef enum {
    MODE_NUMERIC = 0x1,
    MODE_ALPHANUMERIC = 0x2,
    MODE_BYTE = 0x4,
    MODE_KANJI = 0x8
} Mode;

typedef enum {
    VERSION_CLASS_SMALL = 0,
    VERSION_CLASS_MEDIUM = 1,
    VERSION_CLASS_LARGE = 2
} VersionClass;

#endif /* TYPEDEFS_H */
