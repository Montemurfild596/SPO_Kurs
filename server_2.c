#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>

#define SHM_SIZE 1024

int shmid;
void *shared_memory;
pthread_mutex_t lock;

void *handle_client(void *arg) {
    int num = *(int*)arg;
    pthread_mutex_lock(&lock);
    *(int*)shared_memory += num;
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main() {
    key_t key = ftok("shmfile", 65);
    shmid = shmget(key, SHM_SIZE, 0666|IPC_CREAT);
    shared_memory = shmat(shmid, NULL, 0);
    pthread_mutex_init(&lock, NULL);

    while (1) {
        int num;
        printf("Enter a number: ");
        scanf("%d", &num);
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, &num);
    }

    shmdt(shared_memory);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
