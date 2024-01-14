#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#define SHM_KEY 1234
#define BUFFER_SIZE sizeof(struct Message)

struct Message {
    int number;
    int sum;
};

int main() {
    sem_t semaphore;
    struct Message *shared_memory;
    int shmid;

    if (sem_init(&semaphore, 0, 1) < 0) {
        perror("semaphore initialization failed");
        exit(EXIT_FAILURE);
    }

    // Получаем доступ к общей памяти
    shmid = shmget(SHM_KEY, BUFFER_SIZE, 0666);
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

    while (1) {
        printf("Введите число: ");
        scanf("%d", &shared_memory->number);

        sem_wait(&semaphore);

        printf("Число отправлено серверу\n");

        sem_post(&semaphore);
    }

    sem_destroy(&semaphore);

    // Отсоединяем общую память
    shmdt(shared_memory);

    return 0;
}
