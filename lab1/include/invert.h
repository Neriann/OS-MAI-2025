#ifndef INVERT_H
#define INVERT_H

#include <string.h>

#define BUFFER_SIZE 4096

static inline char* reverse(const char* str, char* buff) {
    size_t len = strlen(str);
    for (int i = 0; i < len; ++i) {
        buff[i] = str[len - i - 1];
    }
    buff[len] = 0;
    return buff;
}

#endif //INVERT_H
