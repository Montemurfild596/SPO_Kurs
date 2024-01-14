#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#define SHM_SIZE 1024

int *shared_memory;
sem_t *semaphore;

int main() {
    // Получение доступа к семафору
    semaphore = sem_open("/my_sem", 0);
    if (semaphore == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    // Получение доступа к общей памяти
    key_t key = ftok("/tmp", 'S');
    int shmid = shmget(key, SHM_SIZE * sizeof(int), 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Подключение общей памяти
    shared_memory = (int*)shmat(shmid, NULL, 0);

    // Запрос данных у пользователя
    int numbers[SHM_SIZE];
    printf("Клиент: Введите %d чисел:\n", SHM_SIZE);
    for (int i = 0; i < SHM_SIZE; ++i) {
        scanf("%d", &numbers[i]);
    }

    // Отправка запроса серверу
    sem_wait(semaphore);
    for (int i = 0; i < SHM_SIZE; ++i) {
        shared_memory[i] = numbers[i];
    }
    sem_post(semaphore);

    // Ожидание ответа от сервера
    sleep(1);

    // Отключение от общей памяти
    shmdt(shared_memory);

    return 0;
}
