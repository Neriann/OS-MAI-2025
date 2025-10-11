#ifndef GENERATE_TEST_H
#define GENERATE_TEST_H

#include <stdint.h>

int64_t get_random_number(int64_t start, int64_t end);

int64_t** generate_random_data(int64_t rows_matrix, int64_t cols_matrix, int64_t min_value, int64_t max_value);

int64_t** get_base_data(int64_t* rows_matrix, int64_t* cols_matrix);

#endif
