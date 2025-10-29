#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <unistd.h>
#include <fcntl.h>
#include <wait.h>

#include <sys/mman.h>
#include <semaphore.h>
#include <stdbool.h>

#include "invert.h"

#define FILE_LEN 50
#define SHM_SIZE 4096 // 4KB


int main() {
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

    char shm_name1[FILE_LEN+8], shm_name2[FILE_LEN+8];
    char sem_mutex1[FILE_LEN+8], sem_data1[FILE_LEN+8];
    char sem_mutex2[FILE_LEN+8], sem_data2[FILE_LEN+8];

    // создаем имена объектов
    make_name(filename1, shm_name1, sizeof(shm_name1), "_shm");
    make_name(filename1, sem_mutex1, sizeof(sem_mutex1), "_mut");
    make_name(filename1, sem_data1, sizeof(sem_data1), "_data");

    make_name(filename2, shm_name2, sizeof(shm_name2), "_shm");
    make_name(filename2, sem_mutex2, sizeof(sem_mutex2), "_mut");
    make_name(filename2, sem_data2, sizeof(sem_data2), "_data");

    // инициализируем память файлов как shared, создавая динамические объекты для каждого файла
    int shm_fd1 = shm_open(shm_name1, O_RDWR | O_CREAT | O_TRUNC, 0600);
    int shm_fd2 = shm_open(shm_name2, O_RDWR | O_CREAT | O_TRUNC, 0600);

    if (shm_fd1 == -1 || shm_fd2 == -1) {
        const char msg[] = "shm_open() failed\n";
        write(STDERR_FILENO, msg, strlen(msg));
        exit(EXIT_FAILURE);
    }
    // задаем размер shared memory
    if (ftruncate(shm_fd1, SHM_SIZE) == -1 || ftruncate(shm_fd2, SHM_SIZE) == -1) {
        const char msg[] = "ftruncate() failed\n";
        write(STDERR_FILENO, msg, strlen(msg));
        exit(EXIT_FAILURE);
    }
    // проецируем shared memory в виртуальное адресное пространство процессов
    char* shm_buff1 = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd1, 0);
    char* shm_buff2 = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd2, 0);
    if (shm_buff1 == MAP_FAILED || shm_buff2 == MAP_FAILED) {
        const char msg[] = "mmap() failed\n";
        write(STDERR_FILENO, msg, strlen(msg));
        exit(EXIT_FAILURE);
    }


    // создаем именованные семафоры для каждого файла
    sem_t* mut1 = sem_open(sem_mutex1, O_CREAT, 0600, 1); // 1 - начальное значение семафора
    sem_t* data1 = sem_open(sem_data1, O_CREAT, 0600, 0);
    sem_t* mut2 = sem_open(sem_mutex2, O_CREAT, 0600, 1);
    sem_t* data2 = sem_open(sem_data2, O_CREAT, 0600, 0);
    if (mut1 == SEM_FAILED || data1 == SEM_FAILED || mut2 == SEM_FAILED || data2 == SEM_FAILED) {
        const char msg[] = "sem_open() failed\n";
        write(STDERR_FILENO, msg, strlen(msg));
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        execl("./server", "server", filename1, NULL);
        const char msg[] = "exec server failed\n";
        // если сюда попадаем, то это значит что наш процесс не смог развернуться,
        // ведь при exec вся старая программа стирается и разворачивается новая
        write(STDOUT_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    pid_t pid2 = fork();
    if (pid2 == 0) {
        execl("./server", "server", filename2, NULL);
        const char msg[] = "exec server failed\n";
        write(STDOUT_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    // Parent
    bool running = true;
    while (running) {
        char buff[SHM_SIZE - sizeof(uint32_t)];
        ssize_t bytes_read = read(STDIN_FILENO, buff, sizeof(buff));

        if (bytes_read == -1) {
            const char msg[] = "parent read() failed\n";
            write(STDERR_FILENO, msg, strlen(msg));
            exit(EXIT_FAILURE);
        }
        // пропускаем пустые строки
        if (bytes_read == 1 && buff[bytes_read - 1] == '\n') {
            continue;
        }
        // убираем с конца символ переноса строки
        if (bytes_read > 0 && buff[bytes_read - 1] == '\n') {
            buff[--bytes_read] = 0;
        }

        if (bytes_read > 10) {
            // ограничиваем критическую область для потоков процессов, использующих shared memory
            sem_wait(mut1);

            uint32_t* shm_length1 = (uint32_t*)shm_buff1;
            *shm_length1 = bytes_read;
            char* text = shm_buff1 + sizeof(uint32_t);

            memcpy(text, buff, bytes_read);
            sem_post(mut1);

            // даем сигнал о том что данные записаны
            sem_post(data1);
        }
        else if (bytes_read > 0) {
            sem_wait(mut2);

            uint32_t* shm_length2 = (uint32_t*)shm_buff2;
            *shm_length2 = bytes_read;
            char* text = shm_buff2 + sizeof(uint32_t);

            memcpy(text, buff, bytes_read);
            sem_post(mut2);

            // даем сигнал о том что данные записаны
            sem_post(data2);
        }
        else {
            running = false;

            sem_wait(mut1);

            uint32_t* shm_length1 = (uint32_t*)shm_buff1;
            *shm_length1 = UINT32_MAX;

            sem_post(mut1);
            sem_post(data1);

            sem_wait(mut2);

            uint32_t* shm_length2 = (uint32_t*)shm_buff2;
            *shm_length2 = UINT32_MAX;

            sem_post(mut2);
            sem_post(data2);
        }
    }
    // ждем завершения дочерних процессов
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    // помечаем именованные семафоры на удаление (семафор удалится, когда все процессы закроют его)
    sem_unlink(sem_mutex1);
    sem_unlink(sem_data1);
    sem_unlink(sem_mutex2);
    sem_unlink(sem_data2);

    // освобождаем ресурсы семафора в этом процессе и удаляем его для текущего процесса
    sem_close(mut1);
    sem_close(data1);
    sem_close(mut2);
    sem_close(data2);

    // удаляем отображения в виртуальную память
    munmap(shm_buff1, SHM_SIZE);
    munmap(shm_buff2, SHM_SIZE);

    // помечаем динамические объекты shared memory на удаление (объект удалится, когда все дочерние процессы закроют shared memory)
    shm_unlink(shm_name1);
    shm_unlink(shm_name2);

    // освобождаем файловые дескрипторы и ресурсы семафора
    close(shm_fd1);
    close(shm_fd2);

    exit(EXIT_SUCCESS);
}
