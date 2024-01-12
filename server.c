#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
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
    pthread_mutex_destroy(mutex);
    shmdt(mutex);

    for (int i = 0; i < *num_clients; ++i) {
        free(client_numbers[i]);
    }
    free(client_numbers);

    shmctl(shmget(ftok("/tmp", 'A'), 0, 0666), IPC_RMID, 0); // Удаляем сегмент разделяемой памяти
}

void* handle_client(void* arg) {
    int *client_data = (int*)arg;
    int client_number = client_data[0];

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
    client_numbers = (int**)(shared_sums + 1);
    num_clients = (int*)(client_numbers + 1);

    // Выделение памяти для массива указателей на данные клиентов
    client_numbers[0] = (int*)malloc(sizeof(int));
    num_clients[0] = 0;

    while (1) {
        // Выделение памяти для данных нового клиента
        client_numbers[*num_clients] = (int*)malloc(sizeof(int));
        *(client_numbers[*num_clients]) = *num_clients;

        // Создание нитей для обработки клиентов
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, client_numbers[*num_clients]);

        // Увеличение счетчика клиентов
        (*num_clients)++;
    }

    pthread_mutex_init(mutex, NULL);

    for (int i = 0; i < *num_clients; ++i) {
        pthread_join(thread, NULL);
    }

    printf("Server: Final sums are ");
    for (int i = 0; i < *num_clients; ++i) {
        printf("%d ", shared_sums[i]);
    }
    printf("\n");

    atexit(cleanup);

    return 0;
}
