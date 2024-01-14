#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#define SHM_SIZE  sizeof(int)

int *shared_memory;
sem_t *sem;

int main() {
    key_t key = ftok("shared_memory_key", 'R');
    int shmid = shmget(key, SHM_SIZE, 0666);
    shared_memory = (int *)shmat(shmid, NULL, 0);

    sem = sem_open("/my_semaphore", 0);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    int client_input;
    while (1) {
        printf("Enter a number: ");
        scanf("%d", &client_input);

        sem_wait(sem);  // Ждем, пока семафор не станет доступным

        // Отправка числа на сервер
        *shared_memory = client_input;

        // Получение результата от сервера
        printf("Client received result from server: %d\n", *shared_memory);

        sem_post(sem);  // Освобождаем семафор

        sleep(1);  // Имитация обработки результата
    }

    sem_close(sem);
    shmdt(shared_memory);

    return 0;
}
