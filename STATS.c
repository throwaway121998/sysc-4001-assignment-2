//
//  STATS.c
//  STATS
//
//  Created by Kyle Horne on 2019-03-18.
//  Copyright © 2019 Kyle Horne. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/types.h>

#include "STATS.h"
#include "semun.h"

static int set_semvalue(void);
static void del_semvalue(void);
static int semaphore_p(void);
static int semaphore_v(void);

static int sem_id;

int main(int argc, const char * argv[]) {
    /* Instantiate the semaphore. */
    sem_id = semget((key_t)1234, 1, 0666 | IPC_CREAT);
    if (!set_semvalue()) {
        fprintf(stderr, "Failed to initialize the semaphore\n");
        exit(EXIT_FAILURE);
    }
    /* Instantiate the shared memory for multiple processes. */
    void *shared_memory = (void *)0;
    struct shared_data *shared_data;
    int shmid;
    srand((unsigned int)getpid());
    shmid = shmget((key_t)1234, sizeof(struct shared_data), 0666 | IPC_CREAT);
    if (shmid == -1) {
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }
    /* Make the shared memory accessible to the program. */
    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1) {
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }
    printf("The shared mem is attached @ location: 0x%X\n", (int)shared_memory);
    /* Instantiate the array from user input. */
    shared_data = (struct shared_data *)shared_memory;
    for(int i = 0; i < SZ; i++) {
        printf("Enter a # for arr[%d]: ", i);
        scanf("%d", &shared_data->B[i]);
    }
    shared_data->is_alive = 1;
    /* Fork SZ - 1 processes. */
    int n_processes = SZ - 1;
    for(int i = 0; i < n_processes; i++) {
        pid_t pid = fork();
        switch(pid) {
            case -1:
                perror("fork failed");
                exit(EXIT_FAILURE);
            case 0:
                /* Child process. */
                while(shared_data->is_alive) {
                    /* Enter the critical section. */
                    if (!semaphore_p()) {
                        exit(EXIT_FAILURE);
                    }
                    /* Swap the elements of the array if and only if arr[i] < arr[i + 1] */
                    if (shared_data->B[i] < shared_data->B[i + 1]) {
                        int temp = shared_data->B[i];
                        shared_data->B[i] = shared_data->B[i + 1];
                        shared_data->B[i + 1] = temp;
                    }
                    /* Exit the critical section. */
                    if (!semaphore_v()) {
                        exit(EXIT_FAILURE);
                    }
                }
                /* Exit the process. */
                exit(0);
                break;
            default:
                /* Parent process. */
                while (shared_data->is_alive) {
                    int is_ok = 1;
                    for(int i = 0; i < SZ; i++) {
                        /* Enter the critical section. */
                        if (!semaphore_p()) {
                            exit(EXIT_FAILURE);
                        }
                        if (shared_data->B[i] < shared_data->B[i + 1]) {
                            is_ok = 0;
                        }
                        /* Exit the critical section. */
                        if (!semaphore_v()) {
                            exit(EXIT_FAILURE);
                        }
                    }
                    if (is_ok) {
                        shared_data->is_alive = 0;
                    }
                }
                break;
        }
    }
    /* Wait for child processes to exit. */
    while (n_processes > 0) {
        wait(NULL);
        --n_processes;
    }
    /* Print the reconstructed array. */
    printf("Result: [");
    for(int i = 0; i < SZ; i++) {
        if (i == SZ - 1) {
            printf("%d]\n", shared_data->B[i]);
        } else {
            printf("%d, ", shared_data->B[i]);
        }
    }
    /* Print the min, median and max elements in the array. */
    printf("Min value: %d\n", shared_data->B[SZ - 1]);
    printf("Median value: %d\n", shared_data->B[SZ / 2]);
    printf("Max value: %d\n", shared_data->B[0]);
    /* Delete the semaphore. */
    del_semvalue();
    /* Detach and delete the shared memory. */
    if (shmdt(shared_memory) == -1) {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }
    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        fprintf(stderr, "shmctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}

static int set_semvalue(void) {
    union semun sem_union;
    sem_union.val = 1;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) {
        return(0);
    }
    return(1);
}

static void del_semvalue(void) {
    union semun sem_union;
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1) {
        fprintf(stderr, "Failed to delete the semaphore\n");
    }
}

static int semaphore_p(void) {
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1;
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_p failed\n");
        return(0);
    }
    return(1);
}

static int semaphore_v(void) {
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1;
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_v failed\n");
        return(0);
    }
    return(1);
}
