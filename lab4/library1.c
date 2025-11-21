#include <stdlib.h>

#include "contracts.h"

// First realization of functions


void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

// Euclid's algorithm
int gcd(int a, int b) {
    while (b) {
        a = a % b;
        swap(&a, &b);
    }
    return a;
}

// Convert to binary integer system
char* convert(int x) {
    if (!x) {
        char* str = (char*)malloc(sizeof(char) + 1);
        str[0] = '0';
        str[1] = 0;
        return str;
    }
    int n = 8 * sizeof(int);
    char* str = (char*) malloc(n + 1);

    int sz = 0;
    for (int i = n - 1; i >= 0; --i) {
        if (x & (1U << i)) {
            sz = i + 1;
            break;
        }
    }
    unsigned int ux = (unsigned int)x;
    for (int i = 0; i < sz; ++i) {
        str[i] = (ux & (1 << (sz - i - 1))) > 0 ? '1' : '0';
    }
    str[sz] = 0;
    return str;
}
