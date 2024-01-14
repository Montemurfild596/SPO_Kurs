#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <semaphore.h>

#define SHM_SIZE  sizeof(int)

sem_t mutex;

void *process_request(void *arg) {
    int *shared_data = (int *)arg;

    // Блокировка критической секции
    sem_wait(&mutex);

    // Обработка запроса
    *shared_data += *shared_data;

    // Разблокировка критической секции
    sem_post(&mutex);

    pthread_exit(NULL);
}

int main() {
    key_t key = ftok("shared_memory_key", 65);
    int shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    int *shared_data = (int *)shmat(shmid, NULL, 0);

    // Инициализация семафора
    sem_init(&mutex, 0, 1);

    while (1) {
        pthread_t thread;

        // Принимаем соединение от клиента
        printf("Ждем запрос от клиента...\n");

        // Запуск процесса обработки запроса в отдельном потоке
        pthread_create(&thread, NULL, process_request, (void *)shared_data);

        // Ждем завершения потока
        pthread_join(thread, NULL);

        // Отправка ответа клиенту
        printf("Отправляем ответ клиенту: %d\n", *shared_data);
    }

    // Освобождение ресурсов
    shmdt(shared_data);
    shmctl(shmid, IPC_RMID, NULL);

    sem_destroy(&mutex);

    return 0;
}
