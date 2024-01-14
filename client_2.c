#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHM_KEY 123456
#define SEM_KEY 654321

int main() {
    int shmid, semid;
    int *shared_memory;
    struct sembuf sem_op;

    // Получаем доступ к разделяемой памяти
    if ((shmid = shmget(SHM_KEY, sizeof(int), 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    // Получаем доступ к разделяемой памяти
    if ((shared_memory = shmat(shmid, NULL, 0)) == (int *) -1) {
        perror("shmat");
        exit(1);
    }

    // Получаем доступ к семафору
    if ((semid = semget(SEM_KEY, 1, 0666)) < 0) {
        perror("semget");
        exit(1);
    }

    // Вводим число
    int number;
    printf("Enter a number: ");
    scanf("%d", &number);

    // Записываем число в разделяемую память
    sem_op.sem_num = 0;
    sem_op.sem_op = -1;
    sem_op.sem_flg = 0;
    semop(semid, &sem_op, 1);
    *shared_memory = number;
    sem_op.sem_op = 1;
    semop(semid, &sem_op, 1);

    // Освобождаем разделяемую память
    if (shmdt(shared_memory) < 0) {
        perror("shmdt");
        exit(1);
    }

    return 0;
}
