#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE (sizeof(pthread_mutex_t) + sizeof(int) + sizeof(int[MAX_CLIENTS]))

pthread_mutex_t *mutex;
int *shared_sums;

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

    while (1) {
        pthread_mutex_lock(mutex);

        // Ввод числа с клавиатуры
        int number;
        printf("Client: Enter a number (0 to exit): ");
        scanf("%d", &number);

        // Отправляем число серверу
        printf("Client: Sending number %d to server\n", number);

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
