#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE 1024

int *shared_memory;

void *server_thread(void *arg) {
    int sum = 0;
    
    while (1) {
        // Ждем, пока клиент отправит число
        while (*shared_memory == 0) {
            usleep(100000); // Подождем 100 мс
        }

        // Прибавляем число к общей сумме
        sum += *shared_memory;

        // Обнуляем значение в общей памяти
        *shared_memory = 0;

        // Отправляем сумму клиенту
        *shared_memory = sum;
    }
}

int main() {
    key_t key = ftok("/tmp", 'A');
    int shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    shared_memory = (int *)shmat(shmid, NULL, 0);
    
    if ((intptr_t)shared_memory == -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Создаем нить для обработки клиентских запросов
    pthread_t tid;
    if (pthread_create(&tid, NULL, server_thread, NULL) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    // Основной цикл сервера
    while (1) {
        usleep(100000); // Подождем 100 мс
    }

    return 0;
}
