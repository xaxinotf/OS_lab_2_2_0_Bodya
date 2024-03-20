#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define TOTAL_PHILOSOPHERS 5
#define EATING_LIMIT 10

// Global mutexes for each fork
pthread_mutex_t forkLocks[TOTAL_PHILOSOPHERS];
// Threads representing philosophers
pthread_t philosophers[TOTAL_PHILOSOPHERS];

// Function prototypes
void performAction(int id, const char* actionDescription);
void* philosopherLife(void* id);

int main() {
    srand(time(NULL)); // Initialize random seed

    // Initialize mutexes for fork locks
    for (int i = 0; i < TOTAL_PHILOSOPHERS; i++) {
        pthread_mutex_init(&forkLocks[i], NULL);
    }

    // Create philosopher threads
    int philosopherIds[TOTAL_PHILOSOPHERS];
    for (int i = 0; i < TOTAL_PHILOSOPHERS; i++) {
        philosopherIds[i] = i;
        if (pthread_create(&philosophers[i], NULL, philosopherLife, &philosopherIds[i])) {
            fprintf(stderr, "Failed to create philosopher thread #%d\n", i);
            return EXIT_FAILURE;
        }
    }

    // Wait for all philosopher threads to finish
    for (int i = 0; i < TOTAL_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }

    // Clean up mutexes
    for (int i = 0; i < TOTAL_PHILOSOPHERS; i++) {
        pthread_mutex_destroy(&forkLocks[i]);
    }

    return EXIT_SUCCESS;
}

// Simulates the action of a philosopher (thinking or eating)
void performAction(int id, const char* actionDescription) {
    printf("Philosopher #%d %s.\n", id, actionDescription);
    usleep((1 + rand() % 2) * 100000); // Random delay to simulate action time
}

// Represents the life cycle of a philosopher
void* philosopherLife(void* id) {
    int philosopherId = *(int*)id;
    int mealsHad = 0;

    while (mealsHad < EATING_LIMIT) {
        performAction(philosopherId, "is thinking");

        int leftFork = philosopherId;
        int rightFork = (philosopherId + 1) % TOTAL_PHILOSOPHERS;

        // Attempt to pick up the left fork
        pthread_mutex_lock(&forkLocks[leftFork]);
        performAction(philosopherId, "picked up left fork");

        // Attempt to pick up the right fork
        if (pthread_mutex_trylock(&forkLocks[rightFork]) == 0) {
            performAction(philosopherId, "picked up right fork and starts eating");
            mealsHad++;
            pthread_mutex_unlock(&forkLocks[rightFork]); // Put down the right fork
            performAction(philosopherId, "put down right fork");
        } else {
            // If unable to pick up the right fork, put down the left fork
            performAction(philosopherId, "could not pick up right fork and continues thinking");
        }
        pthread_mutex_unlock(&forkLocks[leftFork]); // Put down the left fork
        performAction(philosopherId, "put down left fork");

        // Exit loop once the eating limit is reached
        if (mealsHad >= EATING_LIMIT) {
            printf("Philosopher #%d is full and leaves the table.\n", philosopherId);
            break;
        }
    }

    return NULL;
}
