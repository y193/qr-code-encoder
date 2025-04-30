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

#include "segment.h"
#include <stdlib.h>

Segment *newSegment(Mode mode, size_t length) {
    if (length == 0) {
        return NULL;
    }

    Segment *segment = (Segment *)malloc(sizeof(Segment));

    if (segment == NULL) {
        return NULL;
    }

    segment->mode = mode;
    segment->length = length;
    segment->next = NULL;

    return segment;
}

Segment *addSegment(Segment *segments, Segment *segment) {
    if (segments == NULL) {
        return segment;
    }

    Segment *p = segments;

    while (p->next != NULL) {
        p = p->next;
    }

    p->next = segment;

    return segments;
}

void freeSegments(Segment *segments) {
    while (segments != NULL) {
        Segment *next = segments->next;
        free(segments);
        segments = next;
    }
}
