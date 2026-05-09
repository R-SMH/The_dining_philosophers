// Name: Sehat Mahde, Shreyastha Banik
// Date: 5/8/2026
// Description: This Program is an implementation of the classic Dining Philosophers problem using threads and semaphores in C. 
// The program simulates a group of philosophers who alternately think and eat. 
// Each philosopher needs two forks to eat, and the program ensures that they can eat without causing a deadlock. 
// The number of philosophers and the number of times each philosopher eats can be specified as command-line arguments. 


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>


#define THINKING 0
#define HUNGRY 1
#define EATING 2


int numOfPhils; // number of phils, passed in as argument[1]
int num_of_eating_times; // number of times to eat each, passed in as argument[2]
sem_t *forks;
sem_t printLock;
int *state;
int *phils;
int *eatCount;

// func to print the state of philosopher
void printState(int id) {
    sem_wait(&printLock);
    if (state[id] == THINKING) {
        printf("Philosopher %d is thinking...\n", id);
    } 
    else if (state[id] == HUNGRY) {
        printf("Philosopher %d is hungry...\n", id);
    } 
    else if (state[id] == EATING) {
        printf("Philosopher %d is eating...\n", id);
    }
    sem_post(&printLock);
}


// functions that may be helpful to create
void test(); // used to check state of philsopher and state of each Fork
             // if philospher is hungry and both left and right are satisifed
             // then they should be able to eat now

// helper func
int leftFork(int id) {
    return (id + numOfPhils - 1) % numOfPhils;
}

int rightFork(int id) {
    return id;
}

void pickupForks(int id) {
    int left = leftFork(id);
    int right = rightFork(id);

    state[id] = HUNGRY;
    printState(id);

    if (id % 2 == 0) {
        sem_wait(&forks[right]);
        sem_wait(&forks[left]);
    } 
    else {
        sem_wait(&forks[left]);
        sem_wait(&forks[right]);
    }

    state[id] = EATING;
    printState(id);
} // waits to grab forks for philospher (denotes when philospher is hungry)


void putDownForks(int id) {
    int left = leftFork(id);
    int right = rightFork(id);

    sem_post(&forks[left]);
    sem_post(&forks[right]);

    state[id] = THINKING;
    printState(id);
} // puts forks back down (denotes when philospher is thinking)


void *philosopher(void *arg) {
    int id = *(int *)arg;

    for (int i = 0; i < num_of_eating_times; i++) {

        state[id] = THINKING;
        printState(id);
        sleep(1);
        pickupForks(id);
        sleep(1);
        putDownForks(id);
    }

    printf("Philosopher %d is done eating %d times\n", id, num_of_eating_times);
    return NULL;
}


int main(int argc, char *argv[]) {

    // error handling: if not enough args are provided
    if (argc != 3) {
        printf("Usage: %s <number_of_philosophers> <times_to_eat>\n", argv[0]);
        return 1;
    }

    numOfPhils = atoi(argv[1]);
    num_of_eating_times = atoi(argv[2]);

    // error handling: if number of philosopher is less than 2 and they are not allowed to eat
    if (numOfPhils <= 1 || num_of_eating_times <= 0) {
        printf("Error: number of philosophers must be more than 1 and times to eat should be positive.\n");
        return 1;
    }

    srand(time(NULL));

    // memory for threads, fork count, their state, and philosophers, and eat count
    pthread_t *threads = malloc(numOfPhils * sizeof(pthread_t));
    forks = malloc(numOfPhils * sizeof(sem_t));
    state = malloc(numOfPhils * sizeof(int));
    phils = malloc(numOfPhils * sizeof(int));
    eatCount = malloc(numOfPhils * sizeof(int));

    // error handling: if anything is missing
    if (threads == NULL || forks == NULL || state == NULL || phils == NULL || eatCount == NULL) {
        printf("Error: memory allocation failed.\n");
        return 1;
    }


    sem_init(&printLock, 0, 1);

    // initializing fork count, philosophers, and their state
    for (int i = 0; i < numOfPhils; i++) {
        phils[i] = i;
        state[i] = rand() % 2;
        eatCount[i] = 0;

        sem_init(&forks[i], 0, 1);

        printState(i);
        // printf("Initialized fork %d\n", i);     // comment this out later
    }

    // creating threads per philosopher
    for (int i = 0; i < numOfPhils; i++) {
        pthread_create(&threads[i], NULL, philosopher, &phils[i]);
    }

    // This makes main() wait until all philosopher threads finish
    for (int i = 0; i < numOfPhils; i++) {
        pthread_join(threads[i], NULL);
    }

    // destroying semaphores
    for (int i = 0; i < numOfPhils; i++) {
        sem_destroy(&forks[i]);
    }

    sem_destroy(&printLock);

    free(threads);
    free(forks);
    free(state);
    free(phils);

    return 0;
}