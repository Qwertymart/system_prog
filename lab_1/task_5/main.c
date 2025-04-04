#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define SEM_KEY 1234
#define SHM_KEY 5678
#define MUTEX 0
#define WOMEN 1
#define MEN 2

char string_to_int(const char *str, int *result)
{
    char *endinp;
    *result = strtol(str, &endinp, 10);
    if (*endinp != '\0')
        return 1;
    return 0;
}

void take(int sem_id, int sem)
{
    struct sembuf s = {sem, -1, 0};
    if (semop(sem_id, &s, 1) == -1) {
        perror("semop take failed");
        exit(1);
    }
}

void release(int sem_id, int sem)
{
    struct sembuf s = {sem, 1, 0};
    if (semop(sem_id, &s, 1) == -1) {
        perror("semop release failed");
        exit(1);
    }
}

void man_wants_to_enter(int sem_id, int *women_count, int *men_count, int max_capacity)
{
    take(sem_id, MUTEX);

    while (*women_count > 0) {
        release(sem_id, MUTEX);
        take(sem_id, MEN);
        take(sem_id, MUTEX);
    }

    while (*men_count >= max_capacity) {
        release(sem_id, MUTEX);
        take(sem_id, MEN);
        take(sem_id, MUTEX);
    }

    (*men_count)++;
    printf("A man entered. Men in bathroom: %d\n", *men_count);
    release(sem_id, MEN);
    release(sem_id, MUTEX);
}

void woman_wants_to_enter(int sem_id, int *women_count, int *men_count, int max_capacity)
{
    take(sem_id, MUTEX);

    while (*men_count > 0) {
        release(sem_id, MUTEX);
        take(sem_id, WOMEN);
        take(sem_id, MUTEX);
    }

    while (*women_count >= max_capacity) {
        release(sem_id, MUTEX);
        take(sem_id, WOMEN);
        take(sem_id, MUTEX);
    }

    (*women_count)++;
    printf("A woman entered. Women in bathroom: %d\n", *women_count);
    release(sem_id, WOMEN);
    release(sem_id, MUTEX);
}

void man_leaves(int sem_id, int *men_count, int max_capacity)
{
    take(sem_id, MUTEX);

    (*men_count)--;
    printf("A man left. Men in bathroom: %d\n", *men_count);

    if (*men_count < max_capacity) {
        release(sem_id, MEN);
    }

    if (*men_count == 0)
    {
        release(sem_id, WOMEN);
    }

    release(sem_id, MUTEX);
}

void woman_leaves(int sem_id, int *women_count, int max_capacity)
{
    take(sem_id, MUTEX);

    (*women_count)--;
    printf("A woman left. Women in bathroom: %d\n", *women_count);

    if (*women_count == 0) {
        release(sem_id, WOMEN);
    }

    if (*women_count == 0)
    {
        release(sem_id, MEN);
    }

    release(sem_id, MUTEX);
}

void woman_process(int sem_id, int *women_count, int *men_count, int max_capacity)
{
    srand(time(NULL) ^ getpid());

    woman_wants_to_enter(sem_id, women_count, men_count, max_capacity);
    sleep(rand() % 3 + 1);
    woman_leaves(sem_id, women_count, max_capacity);

    exit(0);
}

void man_process(int sem_id, int *women_count, int *men_count, int max_capacity)
{
    srand(time(NULL) ^ getpid());

    man_wants_to_enter(sem_id, women_count, men_count, max_capacity);
    sleep(rand() % 3 + 1);
    man_leaves(sem_id, men_count, max_capacity);

    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Write the maximum number of people allowed in the bathroom\n");
        return 1;
    }

    int max_capacity;
    if (string_to_int(argv[1], &max_capacity)) {
        printf("Invalid input for maximum capacity.\n");
        return 1;
    }

    srand(time(NULL));

    int total_people;
    char temp_total_people[15];
    printf("Total people: ");
    scanf("%s", temp_total_people);

    if (string_to_int(temp_total_people, &total_people))
    {
        printf("Input error\n");
        return 1;
    }

    int sem_id = semget(SEM_KEY, 3, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("semget failed");
        exit(1);
    }

    semctl(sem_id, 0, SETVAL, 1);
    semctl(sem_id, 1, SETVAL, 0);
    semctl(sem_id, 2, SETVAL, 0);

    int shm_id = shmget(SHM_KEY, 2 * sizeof(int), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(1);
    }

    int *counters = (int *)shmat(shm_id, NULL, 0);
    if (counters == (int *)-1) {
        perror("shmat failed");
        exit(1);
    }

    counters[0] = 0;
    counters[1] = 0;

    printf("Generating random number of men and women:\n");

    for (int i = 0; i < total_people; i++) {
        pid_t pid = fork();
        srand(time(NULL) ^ getpid());
        if (pid == 0) {
            if (rand() % 2 == 0) {
                man_process(sem_id, &counters[0], &counters[1], max_capacity);
            } else {
                woman_process(sem_id, &counters[0], &counters[1], max_capacity);
            }
            exit(0);
        } else if (pid < 0) {
            perror("fork failed");
            exit(1);
        }
    }

    for (int i = 0; i < total_people; i++) {
        wait(NULL);
    }

    shmdt(counters);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID, 0);

    printf("All processes completed. Exiting...\n");

    return 0;
}