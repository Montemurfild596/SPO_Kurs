#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE (sizeof(pthread_mutex_t) + sizeof(int) + sizeof(int*) + sizeof(int))

pthread_mutex_t *mutex;
int *shared_sums;
int **client_numbers;
int *num_clients;

void cleanup() {
    shmdt(mutex);
}

int main() {
    key_t key = ftok("/tmp", 'A');
    int shmid;

    if ((shmid = shmget(key, SHM_SIZE, 0666)) < 0) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    if ((mutex = (pthread_mutex_t *)shmat(shmid, NULL, 0)) == (pthread_mutex_t *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    shared_sums = (int *)(mutex + 1);
    client_numbers = (int**)(shared_sums + 1);
    num_clients = (int*)(client_numbers + 1);

    // Выделение памяти для переменной client_number
    int *client_number = (int*)malloc(sizeof(int));

    pthread_mutex_lock(mutex);
    // Выбираем свободный номер клиента
    *client_number = *num_clients;
    pthread_mutex_unlock(mutex);

    while (1) {
        pthread_mutex_lock(mutex);

        // Ввод числа с клавиатуры
        int number;
        printf("Client %d: Enter a number (0 to exit): ", *client_number);
        scanf("%d", &number);

        // Отправляем число серверу
        printf("Client %d: Sending number %d to server\n", *client_number, number);

        pthread_mutex_unlock(mutex);

        // Проверяем, было ли введено число 0 (сигнал завершения)
        if (number == 0) {
            break;
        }

        // Пауза для предотвращения слишком частого обновления
        sleep(1);
    }

    atexit(cleanup);

    return 0;
}
