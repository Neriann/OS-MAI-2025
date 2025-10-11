#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "generate_test.h"

int64_t get_random_number(int64_t start, int64_t end) {
    return rand() % (end - start + 1) + start;
}

int64_t** generate_random_data(int64_t rows_matrix, int64_t cols_matrix, int64_t min_value, int64_t max_value) {
    srand(time(NULL));

    int64_t** data = (int64_t**)malloc(sizeof(int64_t*) * rows_matrix);
    if (!data) {
        printf("Failed to allocate memory for data\n");
        return NULL;
    }
    for (int64_t i = 0; i < rows_matrix; ++i) {
        data[i] = (int64_t*)malloc(sizeof(int64_t) * cols_matrix);
        if (!data[i]) {
            for (int64_t j = 0; j < i; ++j) {
                free(data[j]);
            }
            free(data);
            printf("Failed to allocate memory for data\n");
            return NULL;
        }
        for (int64_t j = 0; j < cols_matrix; ++j) {
            data[i][j] = get_random_number(min_value, max_value);
        }
    }
    return data;
}

int64_t** get_base_data(int64_t* rows_matrix, int64_t* cols_matrix) {
    int64_t tmp[5][5] = {
                        {1, 10, 18, 44, 32},
                        {5, 104, 42, 11, 18},
                        {1, 9, 23, 19, 52},
                        {6, 10, 72, 12, 62},
                        {101, 76, 48, 24, 56}
    };
    *rows_matrix = 5;
    *cols_matrix = 5;
    int64_t** data = (int64_t**)malloc(*rows_matrix * sizeof(int64_t*));
    if (!data) {
        printf("Failed to allocate memory for data\n");
        return NULL;
    }
    for (int64_t i = 0; i < *rows_matrix; ++i) {
        data[i] = (int64_t*)malloc(*cols_matrix * sizeof(int64_t));
        if (!data[i]) {
            printf("Failed to allocate memory for data\n");
            for (int64_t j = 0; j < i; ++j) {
                free(data[j]);
            }
            free(data);
            return NULL;
        }
        for (int64_t j = 0; j < *cols_matrix; ++j) {
            data[i][j] = tmp[i][j];
        }
    }

    return data;
}
