#include <ctype.h>
#include <unistd.h>
#include <wait.h>
#include <stdio.h>
#include <stdlib.h>

#include "invert.h"

#define FILE_LEN 50

int main() {
    int p1[2], p2[2];
    if (pipe(p1) == -1 || pipe(p2) == -1) {
        const char msg[] = "pipe() failed\n";
        write(STDERR_FILENO, msg, strlen(msg));
        exit(EXIT_FAILURE);
    }
    char filename1[FILE_LEN], filename2[FILE_LEN];
    ssize_t n = read(STDIN_FILENO, filename1, FILE_LEN - 1); // -терминирующий ноль
    if (n <= 0) {
        const char msg[] = "read() failed\n";
        write(STDERR_FILENO, msg, strlen(msg));
        exit(EXIT_FAILURE);
    }
    filename1[n - 1] = 0; // без \n
    ssize_t m = read(STDIN_FILENO, filename2, FILE_LEN - 1);
    if (m <= 0) {
        const char msg[] = "read() failed\n";
        write(STDERR_FILENO, msg, strlen(msg));
        exit(EXIT_FAILURE);
    }
    filename2[m - 1] = 0; // без \n


    pid_t pid1 = fork();
    if (pid1 == 0) {
        dup2(p1[0], STDIN_FILENO);
        close(p1[0]);
        close(p1[1]);
        close(p2[0]);
        close(p2[1]);
        execl("./server", "server", filename1, NULL);
        const char msg[] = "exec server failed\n";
        // если сюда попадаем, то это значит что наш процесс не смог развернуться,
        // ведь при exec вся старая программа стирается и разворачивается новая
        write(STDOUT_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    pid_t pid2 = fork();
    if (pid2 == 0) {
        dup2(p2[0], STDIN_FILENO);
        close(p1[0]);
        close(p1[1]);
        close(p2[0]);
        close(p2[1]);
        execl("./server", "server", filename2, NULL);
        const char msg[] = "exec server failed\n";
        write(STDOUT_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    // Parent
    close(p1[0]);
    close(p2[0]);

    ssize_t bytes_read;
    char line[BUFFER_SIZE], buff[BUFFER_SIZE];
    size_t pos = 0;
    while ((bytes_read = read(STDIN_FILENO, line, BUFFER_SIZE - 1))) {
        if (bytes_read < 0) {
            const char msg[] = "read() failed\n";
            write(STDERR_FILENO, msg, strlen(msg));
            exit(EXIT_FAILURE);
        }
        line[bytes_read] = 0;
        for (int i = 0; i <= bytes_read; ++i) {
            if (line[i] == '\n' || line[i] == '\0') {
                buff[i] = 0;
                if (pos > 0) {
                    // чтобы не закидывать пустые строки в ответ
                    size_t len = strlen(buff);
                    if (len > 10) {
                        write(p2[1], buff, len);
                        write(p2[1], "\n", 1);
                    }
                    else {
                        write(p1[1], buff, len);
                        write(p1[1], "\n", 1);
                    }
                }
                pos = 0;
            }
            else {
                buff[pos++] = line[i];
            }
        }
    }
    close(p1[1]);
    close(p2[1]);


    wait(NULL);
    wait(NULL);

    exit(EXIT_SUCCESS);
}
