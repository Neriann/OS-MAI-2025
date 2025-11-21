#ifndef INTERACTIVE_INPUT_H
#define INTERACTIVE_INPUT_H

#include <stdio.h> // snprintf

#include <unistd.h>   // read, write
#include <stdlib.h>   // malloc, free, strtol
#include <string.h>   // strlen


static inline ssize_t putstr(const char *s) {
    return write(1, s, strlen(s));
}

static inline int read_line(char *buf, size_t size) {
    ssize_t n = read(0, buf, size - 1);
    if (n <= 0) return 0;
    buf[n] = '\0';

    // убираем \n
    char *nl = strchr(buf, '\n');
    if (nl) *nl = '\0';

    return 1;
}

static inline int read_int(int *out) {
    char buf[64];
    if (!read_line(buf, sizeof(buf))) return 0;

    char *end;
    long v = strtol(buf, &end, 10);
    if (end == buf) return 0; // не число

    *out = (int)v;
    return 1;
}


#endif // INTERACTIVE_INPUT_H
