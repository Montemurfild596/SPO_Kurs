#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#define SHM_SIZE sizeof(int)

void *thread_func(void *arg) {
    int shm_id = *(int*)arg;
    int *shm_ptr;
    int num;

    // Получаем указатель на разделяемую память
    shm_ptr = shmat(shm_id, NULL, 0);
    if (shm_ptr == (int*)-1) {
        perror("shmat");
        exit(1);
    }

    // Читаем число из разделяемой памяти
    num = *shm_ptr;

    // Записываем сумму всех чисел в разделяемую память
    *shm_ptr += num;

    // Отсоединяемся от разделяемой памяти
    if (shmdt(shm_ptr) == -1) {
        perror("shmdt");
        exit(1);
    }

    return NULL;
}

int main() {
    int shm_id;
    int *shm_ptr;
    pthread_t tid;

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

    // Создаем нить
    pthread_create(&tid, NULL, thread_func, &shm_id);

    // Ждем, пока нить не завершится
    pthread_join(tid, NULL);

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
