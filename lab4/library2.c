#include <stdlib.h>

#include "contracts.h"

int min(int a, int b) {
    return a < b ? a : b;
}

void swap(char* a, char* b) {
    char tmp = *a;
    *a = *b;
    *b = tmp;
}

// Native algorithm
int gcd(int a, int b) {
    int mn = min(a, b);
    for (int d = mn; d > 1; --d) {
        if (a % d == 0 && b % d == 0) {
            return d;
        }
    }
    return 1;
}

// Convert to ternary system
char* convert(int x) {
    int capacity = 16;
    int size = 0;

    char* str = (char*)malloc(capacity * sizeof(char));

    if (!x) {
        str[0] = '0';
        str[1] = 0;
        return str;
    }
    int n = x > 0 ? x : -x;
    while (n) {
        if (size + 1 >= capacity) {
            capacity *= 2;
            char* new_str = (char*)realloc(str, capacity * sizeof(char));
            if (!new_str) {
                free(str); 
                return 0;
            }
            str = new_str;
        }
        str[size++] = '0' + (n % 3);
        n /= 3;
    }

    if (x < 0) {
        if (size + 1 >= capacity) {
            ++capacity;
            char* new_str = (char*)realloc(str, capacity * sizeof(char));
            if (!new_str) {
                free(str); 
                return 0;
            }
            str = new_str;
        }
        str[size++] = '-';
    }

    for (int i = 0; i < size / 2; ++i) {
        swap(&str[i], &str[size - i - 1]);
    }
    str[size] = 0;
    return str;
}
