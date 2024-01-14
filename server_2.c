#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <semaphore.h>

#define SHM_SIZE 1024

int *shared_memory;
sem_t *semaphore;

void* process_request(void* arg) {
    int client_number = *(int*)arg;
    
    // Получение доступа к общей памяти
    sem_wait(semaphore);

    // Обработка запроса
    int sum = 0;
    for (int i = 0; i < SHM_SIZE; ++i) {
        sum += shared_memory[i];
    }

    // Отправка ответа клиенту
    printf("Сервер: Обработка запроса от клиента %d. Ответ: %d\n", client_number, sum);

    // Освобождение ресурсов
    sem_post(semaphore);
    pthread_exit(NULL);
}

int main() {
    // Создание семафора
    semaphore = sem_open("/my_sem", O_CREAT | O_EXCL, 0644, 1);
    if (semaphore == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    // Создание общей памяти
    key_t key = ftok("/tmp", 'S');
    int shmid = shmget(key, SHM_SIZE * sizeof(int), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Подключение общей памяти
    shared_memory = (int*)shmat(shmid, NULL, 0);

    // Основной цикл сервера
    int client_number = 1;
    while (1) {
        pthread_t thread;
        pthread_create(&thread, NULL, process_request, (void*)&client_number);
        pthread_detach(thread);
        client_number++;
    }

    // Освобождение ресурсов
    sem_close(semaphore);
    sem_unlink("/my_sem");
    shmdt(shared_memory);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
