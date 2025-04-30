#include "../src/dataanalysis.h"
#include <assert.h>
#include <stdio.h>

#define SPACE "\x81\x40"

static void test_createModeSegment(void) {
    Segment *segment;

    segment = createModeSegment((const uint8_t *)"1", 1, true);
    assert(segment->mode == MODE_NUMERIC);

    segment = createModeSegment((const uint8_t *)"A", 1, true);
    assert(segment->mode == MODE_ALPHANUMERIC);

    segment = createModeSegment((const uint8_t *)"a", 1, true);
    assert(segment->mode == MODE_BYTE);

    segment = createModeSegment((const uint8_t *)SPACE, 2, true);
    assert(segment->mode == MODE_KANJI);

    segment = createModeSegment((const uint8_t *)SPACE, 2, false);
    assert(segment->mode == MODE_BYTE);

    segment = createModeSegment((const uint8_t *)SPACE "\x81", 3, true);
    assert(segment->mode == MODE_BYTE);

    segment = createModeSegment((const uint8_t *)SPACE SPACE, 4, true);
    assert(segment->mode == MODE_KANJI);

    segment = createModeSegment((const uint8_t *)SPACE "3D", 4, true);
    assert(segment->mode == MODE_BYTE);

    segment = createModeSegment((const uint8_t *)"1B3D", 4, true);
    assert(segment->mode == MODE_ALPHANUMERIC);

    segment = createModeSegment((const uint8_t *)"1B3d", 4, true);
    assert(segment->mode == MODE_BYTE);

    printf("test_createModeSegment() passed\n");
}

static void test_createMixedModeSegments_UseMixedMode(void) {
    Segment *segment;
    const uint8_t *data;

    data = (const uint8_t *)"123456G?";
    segment = createMixedModeSegments(data, 8, false, VERSION_CLASS_SMALL);
    assert(segment->mode == MODE_BYTE);

    data = (const uint8_t *)"123456G?9";
    segment = createMixedModeSegments(data, 9, false, VERSION_CLASS_SMALL);
    assert(segment->mode == MODE_ALPHANUMERIC);

    data = (const uint8_t *)SPACE "?" SPACE SPACE SPACE SPACE SPACE "?";
    segment = createMixedModeSegments(data, 14, true, VERSION_CLASS_LARGE);
    assert(segment->mode == MODE_BYTE);

    data = (const uint8_t *)SPACE "?" SPACE SPACE SPACE SPACE SPACE "??";
    segment = createMixedModeSegments(data, 15, true, VERSION_CLASS_LARGE);
    assert(segment->mode == MODE_KANJI);

    printf("test_createMixedModeSegments_UseMixedMode() passed\n");
}

static void test_selectInitialMode_Numeric(void) {
    Segment *segment;
    const uint8_t *data;

    data = (const uint8_t *)"1234?F789";

    segment = createMixedModeSegments(data, 9, false, VERSION_CLASS_MEDIUM);
    assert(segment->mode == MODE_NUMERIC);

    segment = createMixedModeSegments(data, 9, false, VERSION_CLASS_LARGE);
    assert(segment->mode == MODE_BYTE);

    data = (const uint8_t *)"1234567H9";

    segment = createMixedModeSegments(data, 9, false, VERSION_CLASS_SMALL);
    assert(segment->mode == MODE_NUMERIC);

    segment = createMixedModeSegments(data, 9, false, VERSION_CLASS_MEDIUM);
    assert(segment->mode == MODE_ALPHANUMERIC);

    printf("test_selectInitialMode_Numeric() passed\n");
}

static void test_selectInitialMode_Alphanumeric(void) {
    Segment *segment;
    const uint8_t *data;

    data = (const uint8_t *)"ABCDEF?HI";

    segment = createMixedModeSegments(data, 9, false, VERSION_CLASS_SMALL);
    assert(segment->mode == MODE_ALPHANUMERIC);

    segment = createMixedModeSegments(data, 9, false, VERSION_CLASS_MEDIUM);
    assert(segment->mode == MODE_BYTE);

    data = (const uint8_t *)"ABCDEFG?I";

    segment = createMixedModeSegments(data, 9, false, VERSION_CLASS_MEDIUM);
    assert(segment->mode == MODE_ALPHANUMERIC);

    segment = createMixedModeSegments(data, 9, false, VERSION_CLASS_LARGE);
    assert(segment->mode == MODE_BYTE);

    printf("test_selectInitialMode_Alphanumeric() passed\n");
}

static void test_selectInitialMode_Kanji(void) {
    Segment *segment;
    const uint8_t *data;

    data = (const uint8_t *)SPACE "3456789012345";

    segment = createMixedModeSegments(data, 15, false, VERSION_CLASS_SMALL);
    assert(segment->mode == MODE_BYTE);

    segment = createMixedModeSegments(data, 15, true, VERSION_CLASS_SMALL);
    assert(segment->mode == MODE_KANJI);

    data = (const uint8_t *)SPACE "?" SPACE SPACE SPACE SPACE SPACE "45";

    segment = createMixedModeSegments(data, 15, true, VERSION_CLASS_MEDIUM);
    assert(segment->mode == MODE_BYTE);

    segment = createMixedModeSegments(data, 15, true, VERSION_CLASS_LARGE);
    assert(segment->mode == MODE_KANJI);

    printf("test_selectInitialMode_Kanji() passed\n");
}

static void test_selectInitialMode_Byte(void) {
    Segment *segment;
    const uint8_t *data;

    data = (const uint8_t *)"?23456789";

    segment = createMixedModeSegments(data, 9, false, VERSION_CLASS_SMALL);
    assert(segment->mode == MODE_BYTE);

    printf("test_selectInitialMode_Byte() passed\n");
}

static void test_createMixedModeSegments_ByteToKanji(void) {
    Segment *segment;
    const uint8_t *data;

    data = (const uint8_t *)"?" SPACE SPACE SPACE SPACE SPACE SPACE SPACE SPACE
        SPACE SPACE SPACE SPACE;

    segment = createMixedModeSegments(data, 25, true, VERSION_CLASS_MEDIUM);

    assert(segment->mode == MODE_BYTE);
    assert(segment->length == 1);
    assert(segment->next != NULL);

    segment = segment->next;

    assert(segment->mode == MODE_KANJI);
    assert(segment->length == 24);

    segment = createMixedModeSegments(data, 25, false, VERSION_CLASS_MEDIUM);

    assert(segment->mode == MODE_BYTE);
    assert(segment->length == 25);

    segment = createMixedModeSegments(data, 25, true, VERSION_CLASS_LARGE);

    assert(segment->mode == MODE_BYTE);
    assert(segment->length == 25);

    data = (const uint8_t *)"?2" SPACE SPACE SPACE SPACE SPACE SPACE SPACE SPACE
        SPACE;

    segment = createMixedModeSegments(data, 20, true, VERSION_CLASS_SMALL);

    assert(segment->mode == MODE_BYTE);
    assert(segment->length == 2);
    assert(segment->next != NULL);

    segment = segment->next;

    assert(segment->mode == MODE_KANJI);
    assert(segment->length == 18);

    data = (const uint8_t *)"?BC" SPACE SPACE SPACE SPACE SPACE SPACE SPACE
        SPACE SPACE;

    segment = createMixedModeSegments(data, 21, true, VERSION_CLASS_SMALL);

    assert(segment->mode == MODE_BYTE);
    assert(segment->length == 3);
    assert(segment->next != NULL);

    segment = segment->next;

    assert(segment->mode == MODE_KANJI);
    assert(segment->length == 18);

    printf("test_createMixedModeSegments_ByteToKanji() passed\n");
}

static void test_createMixedModeSegments_ByteToAlphanumeric(void) {
    Segment *segment;
    const uint8_t *data;

    data = (const uint8_t *)"?BCDEFGHIJKLMNOP";
    segment = createMixedModeSegments(data, 16, false, VERSION_CLASS_MEDIUM);

    assert(segment->mode == MODE_BYTE);
    assert(segment->length == 1);
    assert(segment->next != NULL);

    segment = segment->next;

    assert(segment->mode == MODE_ALPHANUMERIC);
    assert(segment->length == 15);

    segment = createMixedModeSegments(data, 16, false, VERSION_CLASS_LARGE);

    assert(segment->mode == MODE_BYTE);
    assert(segment->length == 16);

    data = (const uint8_t *)"?2CDEFGHIJKLM";
    segment = createMixedModeSegments(data, 13, false, VERSION_CLASS_SMALL);

    assert(segment->mode == MODE_BYTE);
    assert(segment->length == 2);
    assert(segment->next != NULL);

    segment = segment->next;

    assert(segment->mode == MODE_ALPHANUMERIC);
    assert(segment->length == 11);

    data = (const uint8_t *)"?" SPACE "DEFGHIJKLMNO";
    segment = createMixedModeSegments(data, 15, true, VERSION_CLASS_SMALL);

    assert(segment->mode == MODE_BYTE);
    assert(segment->length == 3);
    assert(segment->next != NULL);

    segment = segment->next;

    assert(segment->mode == MODE_ALPHANUMERIC);
    assert(segment->length == 12);

    printf("test_createMixedModeSegments_ByteToAlphanumeric() passed\n");
}

static void test_createMixedModeSegments_ByteToNumeric(void) {
    Segment *segment;
    const uint8_t *data;

    data = (const uint8_t *)"?23456789";
    segment = createMixedModeSegments(data, 9, false, VERSION_CLASS_MEDIUM);

    assert(segment->mode == MODE_BYTE);
    assert(segment->length == 1);
    assert(segment->next != NULL);

    segment = segment->next;

    assert(segment->mode == MODE_NUMERIC);
    assert(segment->length == 8);

    segment = createMixedModeSegments(data, 9, false, VERSION_CLASS_LARGE);

    assert(segment->mode == MODE_BYTE);
    assert(segment->length == 1);
    assert(segment->next != NULL);

    segment = segment->next;

    assert(segment->mode == MODE_NUMERIC);
    assert(segment->length == 8);

    data = (const uint8_t *)"?23456789J";
    segment = createMixedModeSegments(data, 10, false, VERSION_CLASS_LARGE);

    assert(segment->mode == MODE_BYTE);
    assert(segment->length == 1);
    assert(segment->next != NULL);

    segment = segment->next;

    assert(segment->mode == MODE_NUMERIC);
    assert(segment->length == 8);

    data = (const uint8_t *)"?23456789?";
    segment = createMixedModeSegments(data, 10, false, VERSION_CLASS_LARGE);

    assert(segment->mode == MODE_BYTE);
    assert(segment->length == 10);

    data = (const uint8_t *)"?B3456789";
    segment = createMixedModeSegments(data, 9, false, VERSION_CLASS_MEDIUM);

    assert(segment->mode == MODE_BYTE);
    assert(segment->length == 2);
    assert(segment->next != NULL);

    segment = segment->next;

    assert(segment->mode == MODE_NUMERIC);
    assert(segment->length == 7);

    data = (const uint8_t *)"?" SPACE "456789JKLMNO";
    segment = createMixedModeSegments(data, 15, true, VERSION_CLASS_SMALL);

    assert(segment->mode == MODE_BYTE);
    assert(segment->length == 3);
    assert(segment->next != NULL);

    segment = segment->next;

    assert(segment->mode == MODE_NUMERIC);
    assert(segment->length == 6);

    printf("test_createMixedModeSegments_ByteToNumeric() passed\n");
}

static void test_createMixedModeSegments_AlphanumericToNumeric(void) {
    Segment *segment;
    const uint8_t *data;

    data = (const uint8_t *)"A2345678901234";
    segment = createMixedModeSegments(data, 14, false, VERSION_CLASS_SMALL);

    assert(segment->mode == MODE_ALPHANUMERIC);
    assert(segment->length == 1);
    assert(segment->next != NULL);

    segment = segment->next;

    assert(segment->mode == MODE_NUMERIC);
    assert(segment->length == 13);

    segment = createMixedModeSegments(data, 14, false, VERSION_CLASS_MEDIUM);

    assert(segment->mode == MODE_ALPHANUMERIC);
    assert(segment->length == 14);

    printf("test_createMixedModeSegments_AlphanumericToNumeric() passed\n");
}

static void test_createMixedModeSegments_ByteToByte(void) {
    Segment *segment;
    const uint8_t *data;

    data = (const uint8_t *)"?2C" SPACE "F7" SPACE "J?" SPACE "?5?";

    segment = createMixedModeSegments(data, 16, true, VERSION_CLASS_SMALL);

    assert(segment->mode == MODE_BYTE);
    assert(segment->length == 16);

    printf("test_createMixedModeSegments_ByteToByte() passed\n");
}

static void test_createMixedModeSegments_AlnumToAlnum(void) {
    Segment *segment;
    const uint8_t *data;

    data = (const uint8_t *)"A23D5678I";
    segment = createMixedModeSegments(data, 9, false, VERSION_CLASS_SMALL);

    assert(segment->mode == MODE_ALPHANUMERIC);
    assert(segment->length == 9);

    printf("test_createMixedModeSegments_AlnumToAlnum() passed\n");
}

static void test_createMixedModeSegments_ChangeMode(void) {
    Segment *segment;
    const uint8_t *data;

    data = (const uint8_t *)SPACE "3" SPACE "F" SPACE "?012345P?";

    segment = createMixedModeSegments(data, 17, true, VERSION_CLASS_SMALL);

    assert(segment->mode == MODE_KANJI);
    assert(segment->length == 2);
    assert(segment->next != NULL);

    segment = segment->next;

    assert(segment->mode == MODE_NUMERIC);
    assert(segment->length == 1);
    assert(segment->next != NULL);

    segment = segment->next;

    assert(segment->mode == MODE_KANJI);
    assert(segment->length == 2);
    assert(segment->next != NULL);

    segment = segment->next;

    assert(segment->mode == MODE_ALPHANUMERIC);
    assert(segment->length == 1);
    assert(segment->next != NULL);

    segment = segment->next;

    assert(segment->mode == MODE_KANJI);
    assert(segment->length == 2);
    assert(segment->next != NULL);

    segment = segment->next;

    assert(segment->mode == MODE_BYTE);
    assert(segment->length == 1);
    assert(segment->next != NULL);

    segment = segment->next;

    assert(segment->mode == MODE_NUMERIC);
    assert(segment->length == 6);
    assert(segment->next != NULL);

    segment = segment->next;

    assert(segment->mode == MODE_ALPHANUMERIC);
    assert(segment->length == 1);
    assert(segment->next != NULL);

    segment = segment->next;

    assert(segment->mode == MODE_BYTE);
    assert(segment->length == 1);

    printf("test_createMixedModeSegments_ChangeMode() passed\n");
}

static void test_createMixedModeSegments_Single(void) {
    Segment *segment;
    const uint8_t *data;

    data = (const uint8_t *)"?????????";
    segment = createMixedModeSegments(data, 9, false, VERSION_CLASS_SMALL);

    assert(segment->mode == MODE_BYTE);
    assert(segment->length == 9);

    data = (const uint8_t *)SPACE SPACE SPACE SPACE SPACE SPACE SPACE SPACE;
    segment = createMixedModeSegments(data, 16, true, VERSION_CLASS_SMALL);

    assert(segment->mode == MODE_KANJI);
    assert(segment->length == 16);

    data = (const uint8_t *)"ABCDEFGHI";
    segment = createMixedModeSegments(data, 9, false, VERSION_CLASS_SMALL);

    assert(segment->mode == MODE_ALPHANUMERIC);
    assert(segment->length == 9);

    data = (const uint8_t *)"123456789";
    segment = createMixedModeSegments(data, 9, false, VERSION_CLASS_SMALL);

    assert(segment->mode == MODE_NUMERIC);
    assert(segment->length == 9);

    printf("test_createMixedModeSegments_Single() passed\n");
}

int main(void) {
    test_createModeSegment();

    test_createMixedModeSegments_UseMixedMode();

    test_selectInitialMode_Numeric();
    test_selectInitialMode_Alphanumeric();
    test_selectInitialMode_Kanji();
    test_selectInitialMode_Byte();

    test_createMixedModeSegments_ByteToKanji();
    test_createMixedModeSegments_ByteToAlphanumeric();
    test_createMixedModeSegments_ByteToNumeric();

    test_createMixedModeSegments_AlphanumericToNumeric();

    test_createMixedModeSegments_ByteToByte();
    test_createMixedModeSegments_AlnumToAlnum();

    test_createMixedModeSegments_ChangeMode();
    test_createMixedModeSegments_Single();

    return 0;
}
