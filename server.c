#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_CLIENTS 5
#define SHM_SIZE (sizeof(pthread_mutex_t) + sizeof(int) + sizeof(int[MAX_CLIENTS]))

pthread_mutex_t *mutex;
int *shared_sums;

void cleanup() {
    pthread_mutex_destroy(mutex);
    shmdt(mutex);
    shmctl(shmget(ftok("/tmp", 'A'), 0, 0666), IPC_RMID, 0); // Удаляем сегмент разделяемой памяти
}

void* handle_client(void* arg) {
    int client_number = *((int*)arg);

    while (1) {
        pthread_mutex_lock(mutex);

        printf("Server: Waiting for data from client %d\n", client_number);

        // Ожидаем, пока клиент передаст число
        int number;
        scanf("%d", &number);

        // Проверяем, было ли передано число 0 (сигнал завершения)
        if (number == 0) {
            pthread_mutex_unlock(mutex);
            break;
        }

        shared_sums[client_number] += number;

        printf("Server: Received number %d from client %d, current sum is %d\n", number, client_number, shared_sums[client_number]);

        pthread_mutex_unlock(mutex);

        // Пауза для предотвращения слишком частого обновления
        sleep(1);
    }

    pthread_exit(NULL);
}

int main() {
    key_t key = ftok("/tmp", 'A');
    int shmid;

    if ((shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666)) < 0) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    if ((mutex = (pthread_mutex_t *)shmat(shmid, NULL, 0)) == (pthread_mutex_t *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    shared_sums = (int *)(mutex + 1);

    pthread_mutex_init(mutex, NULL);

    pthread_t threads[MAX_CLIENTS];

    int client_numbers[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        client_numbers[i] = i;
        pthread_create(&threads[i], NULL, handle_client, &client_numbers[i]);
    }

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        pthread_join(threads[i], NULL);
    }

    printf("Server: Final sums are ");
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        printf("%d ", shared_sums[i]);
    }
    printf("\n");

    atexit(cleanup);

    return 0;
}
