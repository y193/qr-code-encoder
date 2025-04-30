#include "../src/charset.h"
#include <assert.h>
#include <stdio.h>

static void test_getAlphanumericCode(void) {
    assert(getAlphanumericCode('0') == 0);
    assert(getAlphanumericCode('1') == 1);
    assert(getAlphanumericCode('2') == 2);
    assert(getAlphanumericCode('3') == 3);
    assert(getAlphanumericCode('4') == 4);
    assert(getAlphanumericCode('5') == 5);
    assert(getAlphanumericCode('6') == 6);
    assert(getAlphanumericCode('7') == 7);
    assert(getAlphanumericCode('8') == 8);
    assert(getAlphanumericCode('9') == 9);
    assert(getAlphanumericCode('A') == 10);
    assert(getAlphanumericCode('B') == 11);
    assert(getAlphanumericCode('C') == 12);
    assert(getAlphanumericCode('D') == 13);
    assert(getAlphanumericCode('E') == 14);
    assert(getAlphanumericCode('F') == 15);
    assert(getAlphanumericCode('G') == 16);
    assert(getAlphanumericCode('H') == 17);
    assert(getAlphanumericCode('I') == 18);
    assert(getAlphanumericCode('J') == 19);
    assert(getAlphanumericCode('K') == 20);
    assert(getAlphanumericCode('L') == 21);
    assert(getAlphanumericCode('M') == 22);
    assert(getAlphanumericCode('N') == 23);
    assert(getAlphanumericCode('O') == 24);
    assert(getAlphanumericCode('P') == 25);
    assert(getAlphanumericCode('Q') == 26);
    assert(getAlphanumericCode('R') == 27);
    assert(getAlphanumericCode('S') == 28);
    assert(getAlphanumericCode('T') == 29);
    assert(getAlphanumericCode('U') == 30);
    assert(getAlphanumericCode('V') == 31);
    assert(getAlphanumericCode('W') == 32);
    assert(getAlphanumericCode('X') == 33);
    assert(getAlphanumericCode('Y') == 34);
    assert(getAlphanumericCode('Z') == 35);
    assert(getAlphanumericCode(' ') == 36);
    assert(getAlphanumericCode('$') == 37);
    assert(getAlphanumericCode('%') == 38);
    assert(getAlphanumericCode('*') == 39);
    assert(getAlphanumericCode('+') == 40);
    assert(getAlphanumericCode('-') == 41);
    assert(getAlphanumericCode('.') == 42);
    assert(getAlphanumericCode('/') == 43);
    assert(getAlphanumericCode(':') == 44);

    assert(getAlphanumericCode('?') == -1);
    assert(getAlphanumericCode('a') == -1);

    printf("test_getAlphanumericCode() passed\n");
}

static void test_isNumeric(void) {
    assert(!isNumeric('0' - 1));

    assert(isNumeric('0'));
    assert(isNumeric('1'));
    assert(isNumeric('2'));
    assert(isNumeric('3'));
    assert(isNumeric('4'));
    assert(isNumeric('5'));
    assert(isNumeric('6'));
    assert(isNumeric('7'));
    assert(isNumeric('8'));
    assert(isNumeric('9'));

    assert(!isNumeric('9' + 1));

    printf("test_isNumeric() passed\n");
}

static void test_isAlphanumeric(void) {
    assert(isAlphanumeric('0'));
    assert(isAlphanumeric('9'));
    assert(isAlphanumeric('A'));
    assert(isAlphanumeric('Z'));
    assert(isAlphanumeric(' '));
    assert(isAlphanumeric(':'));

    assert(!isAlphanumeric('!'));
    assert(!isAlphanumeric('a'));

    printf("test_isAlphanumeric() passed\n");
}

static void test_isShiftJISKanji(void) {
    assert(isShiftJISKanji(0x80, 0x40) == false);
    assert(isShiftJISKanji(0x81, 0x40) == true);
    assert(isShiftJISKanji(0x9F, 0x40) == true);
    assert(isShiftJISKanji(0xA0, 0x40) == false);

    assert(isShiftJISKanji(0xDF, 0x40) == false);
    assert(isShiftJISKanji(0xE0, 0x40) == true);
    assert(isShiftJISKanji(0xEB, 0x40) == true);
    assert(isShiftJISKanji(0xEC, 0x40) == false);

    assert(isShiftJISKanji(0x81, 0x39) == false);
    assert(isShiftJISKanji(0x81, 0x40) == true);
    assert(isShiftJISKanji(0x81, 0x7E) == true);
    assert(isShiftJISKanji(0x81, 0x7F) == false);
    assert(isShiftJISKanji(0x81, 0x80) == true);
    assert(isShiftJISKanji(0x81, 0xFC) == true);
    assert(isShiftJISKanji(0x81, 0xFD) == false);

    assert(isShiftJISKanji(0xEA, 0xBF) == true);
    assert(isShiftJISKanji(0xEA, 0xC0) == true);

    assert(isShiftJISKanji(0xEB, 0xBF) == true);
    assert(isShiftJISKanji(0xEB, 0xC0) == false);

    printf("test_isShiftJISKanji() passed\n");
}

int main(void) {
    test_getAlphanumericCode();
    test_isNumeric();
    test_isAlphanumeric();
    test_isShiftJISKanji();

    return 0;
}
