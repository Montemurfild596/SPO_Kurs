#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE 1024

int *shared_memory;

int main() {
    key_t key = ftok("/tmp", 'A');
    int shmid = shmget(key, SHM_SIZE, 0666);

    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    shared_memory = (int *)shmat(shmid, NULL, 0);

    if ((intptr_t)shared_memory == -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int number;

        // Получаем число от пользователя
        printf("Введите число: ");
        scanf("%d", &number);

        // Отправляем число серверу
        *shared_memory = number;

        // Ждем, пока сервер посчитает сумму
        while (*shared_memory == 0) {
            usleep(100000); // Подождем 100 мс
        }

        // Выводим результат
        printf("Сумма полученных чисел: %d\n", *shared_memory);
    }

    return 0;
}
