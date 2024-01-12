#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE 1024

pthread_mutex_t *mutex;
int *shared_sum;

void cleanup() {
    pthread_mutex_destroy(mutex);
    shmdt(mutex);
    shmctl(shmget(ftok("/tmp", 'A'), 0, 0666), IPC_RMID, 0); // Удаляем сегмент разделяемой памяти
}

void* handle_clients(void* arg) {
    int* client_number = (int*)arg;

    while (1) {
        pthread_mutex_lock(mutex);

        printf("Server: Waiting for data from client %d\n", *client_number);

        // Ожидаем, пока клиент передаст число
        int number;
        scanf("%d", &number);

        // Проверяем, было ли передано число 0 (сигнал завершения)
        if (number == 0) {
            pthread_mutex_unlock(mutex);
            break;
        }

        *shared_sum += number;

        pthread_mutex_unlock(mutex);

        // Пауза для предотвращения слишком частого обновления
        sleep(1);
    }

    pthread_exit(NULL);
}

int main() {
    key_t key = ftok("/tmp", 'A');
    int shmid;

    if ((shmid = shmget(key, sizeof(pthread_mutex_t) + sizeof(int), IPC_CREAT | 0666)) < 0) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    if ((mutex = (pthread_mutex_t *)shmat(shmid, NULL, 0)) == (pthread_mutex_t *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    shared_sum = (int *)(mutex + 1);

    pthread_mutex_init(mutex, NULL);

    pthread_t thread1, thread2;

    int client1_number = 1;
    int client2_number = 2;

    pthread_create(&thread1, NULL, handle_clients, &client1_number);
    pthread_create(&thread2, NULL, handle_clients, &client2_number);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Server: Final sum is %d\n", *shared_sum);

    atexit(cleanup);

    return 0;
}
