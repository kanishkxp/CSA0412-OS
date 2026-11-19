#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define N 5 // Number of philosophers and chopsticks

HANDLE chopsticks[N];
HANDLE threads[N];
int philosopher_ids[N];

DWORD WINAPI philosopher(void *arg) {
    int id = *((int *)arg);
    int left = id;                 // Left chopstick index
    int right = (id + 1) % N;      // Right chopstick index

    for (int i = 0; i < 3; i++) { // Eat 3 times for demonstration
        // --- THINKING ---
        printf("Philosopher %d is thinking.\n", id + 1);
        Sleep(1000);

        // --- PICKING UP CHOPSTICKS ---
        // To prevent deadlock, odd philosophers pick left first, even pick right first
        if (id % 2 == 1) {
            WaitForSingleObject(chopsticks[left], INFINITE);
            printf("Philosopher %d picked up left chopstick %d.\n", id + 1, left + 1);
            
            WaitForSingleObject(chopsticks[right], INFINITE);
            printf("Philosopher %d picked up right chopstick %d.\n", id + 1, right + 1);
        } else {
            WaitForSingleObject(chopsticks[right], INFINITE);
            printf("Philosopher %d picked up right chopstick %d.\n", id + 1, right + 1);
            
            WaitForSingleObject(chopsticks[left], INFINITE);
            printf("Philosopher %d picked up left chopstick %d.\n", id + 1, left + 1);
        }

        // --- EATING ---
        printf("Philosopher %d is eating.\n", id + 1);
        Sleep(1500);

        // --- PUTTING DOWN CHOPSTICKS ---
        ReleaseSemaphore(chopsticks[left], 1, NULL);
        ReleaseSemaphore(chopsticks[right], 1, NULL);
        printf("Philosopher %d has finished eating and put down chopsticks.\n", id + 1);
    }
    return 0;
}

int main() {
    // Initialize chopsticks as semaphores with initial count 1 (binary semaphore)
    for (int i = 0; i < N; i++) {
        chopsticks[i] = CreateSemaphore(NULL, 1, 1, NULL);
        philosopher_ids[i] = i;
    }

    // Create philosopher threads
    for (int i = 0; i < N; i++) {
        threads[i] = CreateThread(NULL, 0, philosopher, &philosopher_ids[i], 0, NULL);
    }

    // Wait for all philosopher threads to finish execution
    WaitForMultipleObjects(N, threads, TRUE, INFINITE);

    // Cleanup resources
    for (int i = 0; i < N; i++) {
        CloseHandle(threads[i]);
        CloseHandle(chopsticks[i]);
    }

    return 0;
}