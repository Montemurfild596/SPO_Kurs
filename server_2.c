#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <semaphore.h>

#define SHM_KEY 1234
#define BUFFER_SIZE sizeof(struct Message)

sem_t mutex;
int shmid;

struct Message *shared_memory;

void *handle_client(void *arg) {
    while (1) {
        sem_wait(&mutex);

        int number = shared_memory->number;
        printf("Получено число от клиента: %d\n", number);

        shared_memory->sum += number;

        printf("Сумма отправлена клиенту: %d\n", shared_memory->sum);

        sem_post(&mutex);
    }

    pthread_exit(NULL);
}

int main() {
    sem_init(&mutex, 0, 1);

    // Создаем общую память
    shmid = shmget(SHM_KEY, BUFFER_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    // Присоединяем общую память
    shared_memory = (struct Message *)shmat(shmid, NULL, 0);
    if ((int)shared_memory == -1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    pthread_t thread;
    if (pthread_create(&thread, NULL, handle_client, NULL) != 0) {
        perror("pthread_create failed");
        exit(EXIT_FAILURE);
    }

    // Ожидаем завершения потока
    pthread_join(thread, NULL);

    // Отсоединяем общую память
    shmdt(shared_memory);

    // Удаляем общую память
    shmctl(shmid, IPC_RMID, NULL);

    sem_destroy(&mutex);

    return 0;
}
