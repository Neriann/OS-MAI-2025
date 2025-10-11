#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include <pthread.h>
#include <string.h>

#include "generate_test.h"


typedef struct {
    int64_t** matrix;
    int64_t** result;
    int64_t id;
    int64_t n_threads;
    int64_t iterations;
    int64_t rows;
    int64_t cols;
    int64_t window_rows;
    int64_t window_cols;
} ThreadArgs;



int cmp(const void* a, const void* b) {
    int64_t ia = *(int64_t*)a;
    int64_t ib = *(int64_t*)b;
    if (ia < ib)
        return -1;
    if (ia > ib)
        return 1;
    return 0;
}

void app_median_filter(int64_t** matrix, int64_t** result, int64_t rows, int64_t cols, int64_t start_rows, int64_t end_rows,
                       int64_t start_cols,
                       int64_t end_cols, int64_t window_rows, int64_t window_cols) {
    int64_t sz = window_rows * window_cols;
    int64_t* tmp = (int64_t*)malloc(sz * sizeof(int64_t));
    if (!tmp) {
        printf("malloc failed\n");
        return;
    }

    int64_t shift_rows = window_rows / 2;
    int64_t shift_cols = window_cols / 2;

    int64_t median;
    for (int64_t i = start_rows; i < end_rows; ++i) {
        for (int64_t j = start_cols; j < end_cols; ++j) {

            if (i - shift_rows < 0 || i + shift_rows >= rows || j - shift_cols < 0 || j + shift_cols >= cols) {
                // клетки, которые не могут быть центральным значением в окне просто переписываем из исходной матрицы
                result[i][j] = matrix[i][j];
                continue;
            }

            int64_t index = 0;
            int64_t offset_rows = (window_rows % 2 == 0) ? 1 : 0;
            int64_t offset_cols = (window_cols % 2 == 0) ? 1 : 0;

            for (int64_t k = -shift_rows; k <= shift_rows - offset_rows; ++k) {
                for (int64_t l = -shift_cols; l <= shift_cols - offset_cols; ++l) {
                    tmp[index++] = matrix[i + k][j + l];
                }
            }
            qsort(tmp, sz, sizeof(int64_t), cmp);
            if (sz % 2 == 1)
                median = tmp[sz / 2];
            else
                median = (tmp[sz / 2 - 1] + tmp[sz / 2]) / 2; // псевдомедиана

            result[i][j] = median;
        }
    }
    free(tmp);
}

static void* calculate(void* _args) {
    ThreadArgs* args = (ThreadArgs*)_args;

    int64_t chunk_rows = args->rows / args->n_threads;

    int64_t start_rows = args->id * chunk_rows;
    int64_t end_rows = (args->id == args->n_threads - 1) ? args->rows : start_rows + chunk_rows;


    for (int64_t i = 0; i < args->iterations; ++i) {
        app_median_filter(args->matrix, args->result, args->rows, args->cols, start_rows, end_rows, 0, args->cols,
                          args->window_rows,
                          args->window_cols);
    }
    return NULL;
}


double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}


int main(int argc, char** argv) {
    if (argc != 4) {
        printf("Usage: %s <flag> <target_file> <n_threads>\n", argv[0]);
        return 1;
    }
    const char* flag = argv[1];
    const char* filename = argv[2];
    int64_t n_threads = strtol(argv[3], NULL, 10);
    if (n_threads < 1) {
        printf("Invalid number of arguments\n");
        return 1;
    }
    int64_t rows_matrix, cols_matrix, min_value, max_value;

    int64_t** matrix;
    if (strcmp("-r", flag) == 0) {
        printf("<rows_matrix>: ");
        scanf("%lld", &rows_matrix);

        printf("<cols_matrix>: ");
        scanf("%lld", &cols_matrix);

        printf("<min_value>: ");
        scanf("%lld", &min_value);

        printf("<max_value>: ");
        scanf("%lld", &max_value);

        matrix = generate_random_data(rows_matrix, cols_matrix, min_value, max_value);
    } else if (strcmp("-b", flag) == 0) {
        matrix = get_base_data(&rows_matrix, &cols_matrix);
    } else {
        printf("Invalid flag\n");
        return 1;
    }

    int64_t iters, length, width;
    printf("<number of repetitions>: ");
    scanf("%lld", &iters);

    printf("<median filter window length>: ");
    scanf("%lld", &length);

    printf("<median filter window width>: ");
    scanf("%lld", &width);

    if (rows_matrix < 0 || cols_matrix < 0) {
        printf("matrix sides must be positive\n");
        return 1;
    }
    if (length > rows_matrix || width > cols_matrix) {
        printf("Size the window must be less then matrix");
        return 1;
    }

    if (!matrix) {
        printf("Failed to generate data\n");
        return 1;
    }

    int64_t** result = (int64_t**)malloc(sizeof(int64_t*) * rows_matrix);
    if (!result) {
        printf("malloc failed\n");
        return 1;
    }
    for (int64_t i = 0; i < rows_matrix; ++i) {
        result[i] = (int64_t*)malloc(sizeof(int64_t) * cols_matrix);
        if (!result[i]) {
            printf("malloc failed\n");
            for (int64_t j = 0; j < i; ++j) {
                free(result[j]);
            }
            free(result);
            return 1;
        }
    }
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Failed to open file\n");
        for (int64_t i = 0; i < rows_matrix; ++i) {
            free(result[i]);
            free(matrix[i]);
        }
        free(result);
        free(matrix);
        return 1;
    }
    fprintf(f, "First matrix:\n");
    for (int64_t i = 0; i < rows_matrix; ++i) {
        for (int64_t j = 0; j < cols_matrix; ++j) {
            fprintf(f, "%lld ", matrix[i][j]);
        }
        fprintf(f, "\n");
    }

    pthread_t* threads = (pthread_t*)malloc(n_threads * sizeof(pthread_t));

    ThreadArgs* thread_args = (ThreadArgs*)malloc(n_threads * sizeof(ThreadArgs));

    if (!threads || !thread_args) {
        printf("Failed to allocate memory for threads\n");
        for (int64_t i = 0; i < rows_matrix; ++i) {
            free(matrix[i]);
            free(result[i]);
        }
        free(thread_args);
        free(matrix);
        free(result);
        return 1;
    }

    double start = get_time();

    for (int64_t i = 0; i < n_threads; ++i) {
        thread_args[i] = (ThreadArgs){
            .matrix = matrix,
            .result = result,
            .id = i,
            .n_threads = n_threads,
            .iterations = iters,
            .rows = rows_matrix,
            .cols = cols_matrix,
            .window_rows = length,
            .window_cols = width
        };

        pthread_create(&threads[i], NULL, calculate, &thread_args[i]);
    }

    for (int64_t i = 0; i < n_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    double end = get_time();

    fprintf(f, "Second matrix:\n");
    for (int64_t i = 0; i < rows_matrix; ++i) {
        for (int64_t j = 0; j < cols_matrix; ++j) {
            fprintf(f, "%lld ", result[i][j]);
        }
        fprintf(f, "\n");
    }
    printf("Total running time on %ld threads: %.3lf \n", n_threads, end - start);

    for (int64_t i = 0; i < rows_matrix; ++i) {
        free(matrix[i]);
        free(result[i]);
    }
    free(thread_args);
    free(threads);
    free(result);
    free(matrix);

    fclose(f);
    return 0;
}
