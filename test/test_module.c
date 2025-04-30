#include "test_module.h"

void initializeMatrix(uint8_t *matrix, const uint8_t *input,
                      size_t symbolSize) {
    for (size_t i = 0; i < symbolSize * symbolSize; i++) {
        matrix[i] = input[i] - '0';
    }
}

bool matrixEquals(const uint8_t *matrix, const uint8_t *expected,
                  size_t symbolSize) {
    for (size_t i = 0; i < symbolSize * symbolSize; i++) {
        if (matrix[i] != expected[i] - '0') {
            return false;
        }
    }

    return true;
}
