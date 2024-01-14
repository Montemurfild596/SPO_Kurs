#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <semaphore.h>

#define SHM_SIZE  sizeof(int)

sem_t mutex;
int client_connected = 0;

void *process_request(void *arg) {
    int *shared_data = (int *)arg;

    while (1) {
        // Ждем, пока клиент подключится
        while (!client_connected) {
            // Приостановим выполнение сервера
            pause();
        }

        // Блокировка критической секции
        sem_wait(&mutex);

        // Обработка запроса
        *shared_data += *shared_data;

        // Разблокировка критической секции
        sem_post(&mutex);

        // Отправка ответа клиенту
        printf("Отправляем ответ клиенту: %d\n", *shared_data);

        // Сбрасываем флаг
        client_connected = 0;
    }
}

int main() {
    key_t key = ftok("shared_memory_key", 65);
    int shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    int *shared_data = (int *)shmat(shmid, NULL, 0);

    // Инициализация семафора
    sem_init(&mutex, 0, 1);

    pthread_t thread;

    // Запуск потока для обработки запросов
    pthread_create(&thread, NULL, process_request, (void *)shared_data);

    while (1) {
        // Ожидаем подключения клиента
        printf("Ждем подключения клиента...\n");
        client_connected = 1;

        // После подключения клиента возобновим выполнение сервера
        kill(getpid(), SIGCONT);

        // Пауза для синхронизации
        sleep(1);
    }

    // Освобождение ресурсов
    shmdt(shared_data);
    shmctl(shmid, IPC_RMID, NULL);

    sem_destroy(&mutex);

    return 0;
}
