#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#define SHM_SIZE sizeof(int)

int main() {
    int shm_id;
    int *shm_ptr;
    int num;

    // Создаем разделяемую память
    shm_id = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(1);
    }

    // Получаем указатель на разделяемую память
    shm_ptr = shmat(shm_id, NULL, 0);
    if (shm_ptr == (int*)-1) {
        perror("shmat");
        exit(1);
    }

    // Читаем число от пользователя
    printf("Enter a number: ");
    scanf("%d", &num);

    // Записываем число в разделяемую память
    *shm_ptr = num;

    // Ждем, пока сервер не запишет сумму всех чисел в разделяемую память
    while (*shm_ptr == num) {
        sleep(1);
    }

    // Читаем сумму всех чисел из разделяемой памяти
    printf("Sum of all numbers: %d\n", *shm_ptr);

    // Отсоединяемся от разделяемой памяти
    if (shmdt(shm_ptr) == -1) {
        perror("shmdt");
        exit(1);
    }

    // Удаляем разделяемую память
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }

    return 0;
}
