#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE  sizeof(int)

int main() {
    key_t key = ftok("shared_memory_key", 65);
    int shmid = shmget(key, SHM_SIZE, 0666);
    int *shared_data = (int *)shmat(shmid, NULL, 0);

    // Запрос у пользователя
    int user_input;
    printf("Введите число: ");
    scanf("%d", &user_input);

    // Отправка запроса серверу
    *shared_data = user_input;

    // Ожидание ответа от сервера
    sleep(1); // Пауза для синхронизации

    // Вывод ответа
    printf("Ответ от сервера: %d\n", *shared_data);

    // Освобождение ресурсов
    shmdt(shared_data);

    return 0;
}
