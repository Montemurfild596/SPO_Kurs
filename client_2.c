#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE 1024

int main() {
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, SHM_SIZE, 0666|IPC_CREAT);
    int *shared_memory = (int*)shmat(shmid, NULL, 0);

    while (1) {
        int num;
        printf("Enter a number: ");
        scanf("%d", &num);
        *shared_memory = num;
        printf("Sum: %d\n", *shared_memory);
    }

    shmdt(shared_memory);
    return 0;
}
