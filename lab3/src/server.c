#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/mman.h> // memory management
#include <semaphore.h>


#include "invert.h"

#define SHM_SIZE 4096 // 4KB

int main(int argc, char* argv[]) {
    if (argc < 2) {
        const char msg[] = "Not enough arguments\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(1);
    }
    char* filename = argv[1];

    int fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0600);
    if (fd == -1) {
        const char msg[] = "Error opening file\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(1);
    }
    // создаем имена объектов
    char shm_name[128], sem_mutex_name[128], sem_data_name[128];
    make_name(filename, shm_name, sizeof(shm_name), "_shm");
    make_name(filename, sem_mutex_name, sizeof(sem_mutex_name), "_mut");
    make_name(filename, sem_data_name, sizeof(sem_data_name), "_data");

    // открываем уже созданную shared memory
    int shm_fd = shm_open(shm_name, O_RDWR, 0600);
    if (shm_fd == -1) {
        const char msg[] = "Child error opening file\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(1);
    }

    // проецируем shared memory в виртуальное адресное пространство процесса
    char* shm_buff = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_buff == MAP_FAILED) {
        const char msg[] = "Child mmap failed\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    // открываем существующий семафор
    sem_t* mut = sem_open(sem_mutex_name, 0);
    sem_t* data = sem_open(sem_data_name, 0);
    if (mut == SEM_FAILED || data == SEM_FAILED) {
        const char msg[] = "Child sem_open failed\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    bool running = true;
    while (running) {

        // ждем пока данные будут записаны родителем (аналог блокирующего чтения из pipe)
        sem_wait(data);

        // ограничиваем критическую секцию
        sem_wait(mut);
        // считываем из начала указателя размер shared_memory (от родительского процесса)
        uint32_t* length = (uint32_t*)shm_buff;
        char* text = shm_buff + sizeof(uint32_t);

        if (*length == UINT32_MAX) { // сигнал об окончании ввода
            running = false;
        } else if (*length > 0) {
            char inv_buff[SHM_SIZE - sizeof(uint32_t)];
            // Инвертируем строку
            char* res = reverse(text, *length, inv_buff, sizeof(inv_buff));
            if (res) {
                write(STDOUT_FILENO, res, *length);
                write(STDOUT_FILENO, "\n", 1);
                write(fd, res, *length);
                write(fd, "\n", 1);
                *length = 0;
            } else {
                running = false;
                const char msg[] = "Child reverse failed\n";
                write(STDERR_FILENO, msg, sizeof(msg));
            }
        }
        sem_post(mut);
    }
    // закрываем семафор для текущего процесса и освобождаем его ресурсы
    sem_close(mut); sem_close(data);
    // удаляем отображение, созданное mmap для данного процесса
    munmap(shm_buff, SHM_SIZE);
    // закрываем файловый дескриптор
    close(shm_fd);
    close(fd);
    exit(0);
}
