#ifndef TEST_MODULE_H
#define TEST_MODULE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern void initializeMatrix(uint8_t *matrix, const uint8_t *input,
                             size_t symbolSize);
extern bool matrixEquals(const uint8_t *matrix, const uint8_t *expected,
                         size_t symbolSize);

#endif /* TEST_MODULE_H */
