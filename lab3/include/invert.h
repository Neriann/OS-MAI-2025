#ifndef INVERT_H
#define INVERT_H

#include <string.h>

#define BUFFER_SIZE 4096

static inline char* reverse(const char* str, size_t len, char* buff, size_t buff_size) {
    if (buff_size <= len) return NULL;

    for (int i = 0; i < len; ++i) {
        buff[i] = str[len - i - 1];
    }
    buff[len] = 0;
    return buff;
}

static inline void make_name(const char* base, char* out, size_t out_len, const char* suffix) {
    snprintf(out, out_len, "/%s%s", base, suffix);
}

#endif //INVERT_H
