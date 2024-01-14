#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <pthread.h>

#define SHM_KEY 123456
#define SEM_KEY 654321

int sum = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg) {
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

    // Читаем число из разделяемой памяти
    sem_op.sem_num = 0;
    sem_op.sem_op = -1;
    sem_op.sem_flg = 0;
    semop(semid, &sem_op, 1);
    int number = *shared_memory;
    sem_op.sem_op = 1;
    semop(semid, &sem_op, 1);

    // Обновляем сумму
    pthread_mutex_lock(&lock);
    sum += number;
    printf("Sum: %d\n", sum);
    pthread_mutex_unlock(&lock);

    // Освобождаем разделяемую память
    if (shmdt(shared_memory) < 0) {
        perror("shmdt");
        exit(1);
    }

    return NULL;
}

int main() {
    pthread_t thread;

    while (1) {
        // Создаем новую нить для обработки клиента
        if (pthread_create(&thread, NULL, handle_client, NULL) != 0) {
            perror("pthread_create");
            exit(1);
        }

        // Ждем завершения нити
        if (pthread_join(thread, NULL) != 0) {
            perror("pthread_join");
            exit(1);
        }
    }

    return 0;
}
