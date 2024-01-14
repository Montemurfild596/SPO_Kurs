#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#define SHM_SIZE  sizeof(int)
#define MAX_CLIENTS 5

int *shared_memory;
sem_t *sem;

void *client_handler(void *arg) {
    int client_number = *((int *)arg);

    while (1) {
        int client_input;
        sem_wait(sem);  // Ждем, пока семафор не станет доступным

        printf("Server received from Client %d: %d\n", client_number, *shared_memory);

        // Обработка полученных данных

        // Отправка результата клиенту
        *shared_memory += client_number;

        sem_post(sem);  // Освобождаем семафор

        sleep(1);  // Имитация обработки данных
    }

    pthread_exit(NULL);
}

int main() {
    key_t key = ftok("shared_memory_key", 'R');
    int shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    shared_memory = (int *)shmat(shmid, NULL, 0);

    sem = sem_open("/my_semaphore", O_CREAT | O_EXCL, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    pthread_t threads[MAX_CLIENTS];

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        int *client_number = malloc(sizeof(int));
        *client_number = i + 1;
        pthread_create(&threads[i], NULL, client_handler, (void *)client_number);
    }

    // Ждем завершения всех нитей
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        pthread_join(threads[i], NULL);
    }

    sem_close(sem);
    sem_unlink("/my_semaphore");

    shmdt(shared_memory);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
