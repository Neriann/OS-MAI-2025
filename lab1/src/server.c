#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "invert.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        const char msg[] = "Not enough arguments\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(1);
    }
    char* filename = argv[1];
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0644);
    if (fd == -1) {
        const char msg[] = "Child error opening file\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(1);
    }
    ssize_t bytes_read;
    char line[BUFFER_SIZE], inv_buff[BUFFER_SIZE], buff[BUFFER_SIZE];
    size_t pos = 0;
    while ((bytes_read = read(STDIN_FILENO, line, BUFFER_SIZE - 1))) {
        if (bytes_read < 0) {
            const char msg[] = "read() failed\n";
            write(STDERR_FILENO, msg, strlen(msg));
            exit(EXIT_FAILURE);
        }
        line[bytes_read] = 0;

        // Обрабатываем буфер посимвольно для разбиения на строки
        for (int i = 0; i <= bytes_read; i++) {
            if (line[i] == '\n' || line[i] == 0) {
                buff[pos] = 0;

                if (pos > 0) {
                    // Инвертируем строку
                    char* res = reverse(buff, inv_buff);

                    // Добавляем перенос строки и выводим
                    char output[BUFFER_SIZE];
                    snprintf(output, BUFFER_SIZE, "%s\n", res);
                    size_t len = strlen(output);
                    write(STDOUT_FILENO, output, len);
                    write(fd, output, len);
                }
                pos = 0;
            }
            else {
                buff[pos++] = line[i];
            }
        }
    }
    close(fd);
    exit(0);
}
