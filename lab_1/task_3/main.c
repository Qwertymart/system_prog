#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <unistd.h>

#define NUM_PHILOSOPHERS 5


void sem_op(int semid, int semnum, int op)
{
    struct sembuf sem;
    sem.sem_num = semnum;
    sem.sem_op = op;
    sem.sem_flg = 0;
    semop(semid, &sem, 1);
}

void philosopher(int id, int sem_forks)
{

    printf("Philosopher %d is thinking\n", id);

    sleep(1);
    printf("Philosopher %d wants to eat\n", id);

    if (id % 2 == 0)
    {
        sem_op(sem_forks, id, -1);
        printf("Philosopher %d picked up the right fork %d\n", id, id);
        sem_op(sem_forks, (id + 1) % NUM_PHILOSOPHERS, -1);
        printf("Philosopher %d picked up the left fork %d\n", id, (id + 1) % NUM_PHILOSOPHERS);
    }
    else
    {
        sem_op(sem_forks, (id + 1) % NUM_PHILOSOPHERS, -1);
        printf("Philosopher %d picked up the left fork %d\n", id, (id + 1) % NUM_PHILOSOPHERS);
        sem_op(sem_forks, id, -1);
        printf("Philosopher %d picked up the right fork %d\n", id, id);
    }

    printf("Philosopher %d is eating\n", id);
    sleep(2);

    sem_op(sem_forks, id, 1);
    sem_op(sem_forks, (id + 1) % NUM_PHILOSOPHERS, 1);

    printf("Philosopher %d put down both forks\n", id);
    exit(0);
}



int main()
{
    int i;
    pid_t pids[NUM_PHILOSOPHERS];

    int sem_forks = semget(IPC_PRIVATE, NUM_PHILOSOPHERS, IPC_CREAT | 0666);
    if (sem_forks == -1)
    {
        printf("Semaphore creation failed\n");
        return 1;
    }

    for(i = 0; i < NUM_PHILOSOPHERS; ++i)
    {
        semctl(sem_forks, i, SETVAL, 1);
    }

    for (i = 0; i < NUM_PHILOSOPHERS; ++i)
    {
        pids[i] = fork();
        if (pids[i] == -1)
        {
            printf("Fork failed\n");
            return 2;
        }
        if (pids[i] == 0)
        {
            philosopher(i, sem_forks);
        }
    }
    for (i = 0; i < NUM_PHILOSOPHERS; i++)
    {
        wait(NULL);
    }

    semctl(sem_forks, 0, IPC_RMID);

    return 0;
}