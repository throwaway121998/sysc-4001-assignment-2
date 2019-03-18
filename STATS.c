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

#include <sys/shm.h>
#include <sys/types.h>

#include "STATS.h"
#include "semun.h"

static int set_semvalue(int sem_id);
static int semaphore_p(int sem_id);
static int semaphore_v(int sem_id);

int main(int argc, const char * argv[]) {
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
    int is_debug;
    printf("Would you like to run in debug mode? <0, 1>: ");
    scanf("%d", &is_debug);
    if (is_debug != 0 && is_debug != 1) {
        fprintf(stderr, "invalid mode\n");
        exit(EXIT_FAILURE);
    }
    shared_data = (struct shared_data *)shared_memory;
    for(int i = 0; i < SZ; i++) {
        /* Instantiate the array from user input. */
        printf("Enter a # for arr[%d]: ", i);
        scanf("%d", &shared_data->B[i]);
        /* Instantiate the semaphores. */
        shared_data->mutex[i] = semget((key_t)i /* A unique non-zero key. */, 1, 0666 | IPC_CREAT);
        if (!set_semvalue(shared_data->mutex[i])) {
            fprintf(stderr, "Failed to initialize the semaphore\n");
            exit(EXIT_FAILURE);
        }
    }
    /* Fork SZ processes. */
    int n_processes = SZ - 1;
    for(int i = 0; i < n_processes; i++) {
        pid_t pid = fork();
        switch(pid) {
            case -1:
                perror("fork failed");
                exit(EXIT_FAILURE);
            case 0:
                /* Child process. */
                for(int i = 0; i < SZ; i++) {
                    if (i % 2 == 0) { /* Asymmetric solution. */
                        /* Enter the critical section. */
                        if (!semaphore_p(shared_data->mutex[i])) {
                            exit(EXIT_FAILURE);
                        }
                        if (!semaphore_p(shared_data->mutex[i + 1])) {
                            exit(EXIT_FAILURE);
                        }
                        /* Swap the elements of the array if and only if arr[i] < arr[i + 1] */
                        if (shared_data->B[i] < shared_data->B[i + 1]) {
                            int temp = shared_data->B[i];
                            shared_data->B[i] = shared_data->B[i + 1];
                            shared_data->B[i + 1] = temp;
                            if (is_debug) {
                                printf("Process #%d: performed swapping\n", i);
                            }
                        } else {
                            if (is_debug) {
                                printf("Process #%d: no swapping\n", i);
                            }
                        }
                        /* Exit the critical section. */
                        if (!semaphore_v(shared_data->mutex[i])) {
                            exit(EXIT_FAILURE);
                        }
                        if (!semaphore_v(shared_data->mutex[i + 1])) {
                            exit(EXIT_FAILURE);
                        }
                    } else {
                        /* Enter the critical section. */
                        if (!semaphore_p(shared_data->mutex[i + 1])) {
                            exit(EXIT_FAILURE);
                        }
                        if (!semaphore_p(shared_data->mutex[i])) {
                            exit(EXIT_FAILURE);
                        }
                        /* Swap the elements of the array if and only if arr[i] < arr[i + 1] */
                        if (shared_data->B[i] < shared_data->B[i + 1]) {
                            int temp = shared_data->B[i];
                            shared_data->B[i] = shared_data->B[i + 1];
                            shared_data->B[i + 1] = temp;
                            if (is_debug) {
                                printf("Process #%d: performed swapping\n", i);
                            }
                        } else {
                            if (is_debug) {
                                printf("Process #%d: no swapping\n", i);
                            }
                        }
                        /* Exit the critical section. */
                        if (!semaphore_v(shared_data->mutex[i + 1])) {
                            exit(EXIT_FAILURE);
                        }
                        if (!semaphore_v(shared_data->mutex[i])) {
                            exit(EXIT_FAILURE);
                        }
                    }
                }
                exit(0);
                break;
            default:
                /* Parent process. */
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
            printf("%d, ",  shared_data->B[i]);
        }
    }
    /* Print the min, median and max elements in the array. */
    printf("Min value: %d\n", shared_data->B[SZ - 1]);
    printf("Median value: %d\n", shared_data->B[SZ / 2]);
    printf("Max value: %d\n", shared_data->B[0]);
    /* Detach and delete the shared memory. */
    if (shmdt(shared_memory) == -1) {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }
    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        fprintf(stderr, "shmctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

static int set_semvalue(int sem_id) {
    union semun sem_union;
    sem_union.val = 1;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) {
        return(0);
    }
    return(1);
}

static int semaphore_p(int sem_id) {
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

static int semaphore_v(int sem_id) {
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
