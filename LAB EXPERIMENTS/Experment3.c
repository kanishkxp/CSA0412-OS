#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define BUFFER_SIZE 5
#define TOTAL_ITEMS 10 

// Shared circular buffer and pointers
int buffer[BUFFER_SIZE];
int in = 0;
int out = 0;

// Synchronization handles
HANDLE mutex;
HANDLE empty_slots;
HANDLE full_slots;

// Producer Thread Routine
DWORD WINAPI producer(LPVOID param) {
    for (int i = 1; i <= TOTAL_ITEMS; i++) {
        // 1. Wait until an empty slot is available (decrement empty_slots)
        WaitForSingleObject(empty_slots, INFINITE);

        // 2. Lock buffer (enter critical section)
        WaitForSingleObject(mutex, INFINITE);

        // 3. Add item to buffer
        buffer[in] = i;
        printf("[Producer] Inserted item %d at index %d\n", i, in);
        in = (in + 1) % BUFFER_SIZE;

        // 4. Unlock buffer (leave critical section)
        ReleaseMutex(mutex);

        // 5. Signal that an item is available (increment full_slots)
        ReleaseSemaphore(full_slots, 1, NULL);

        Sleep(150); // Pause 150ms to simulate production time
    }
    return 0;
}

// Consumer Thread Routine
DWORD WINAPI consumer(LPVOID param) {
    for (int i = 1; i <= TOTAL_ITEMS; i++) {
        // 1. Wait until a produced item is available (decrement full_slots)
        WaitForSingleObject(full_slots, INFINITE);

        // 2. Lock buffer (enter critical section)
        WaitForSingleObject(mutex, INFINITE);

        // 3. Remove item from buffer
        int item = buffer[out];
        printf("    [Consumer] Consumed item %d from index %d\n", item, out);
        out = (out + 1) % BUFFER_SIZE;

        // 4. Unlock buffer (leave critical section)
        ReleaseMutex(mutex);

        // 5. Signal that an empty slot is freed (increment empty_slots)
        ReleaseSemaphore(empty_slots, 1, NULL);

        Sleep(300); // Pause 300ms to simulate consumption time
    }
    return 0;
}

int main() {
    HANDLE producer_thread;
    HANDLE consumer_thread;

    // Create a Mutex: initially unowned (FALSE)
    mutex = CreateMutex(NULL, FALSE, NULL);

    // Create Counting Semaphores:
    // empty_slots: initial count = BUFFER_SIZE, max count = BUFFER_SIZE
    empty_slots = CreateSemaphore(NULL, BUFFER_SIZE, BUFFER_SIZE, NULL);
    // full_slots: initial count = 0, max count = BUFFER_SIZE
    full_slots = CreateSemaphore(NULL, 0, BUFFER_SIZE, NULL);

    // Verify handles were successfully created
    if (mutex == NULL || empty_slots == NULL || full_slots == NULL) {
        printf("Error: Synchronization handle creation failed.\n");
        return 1;
    }

    // Launch Producer and Consumer threads
    producer_thread = CreateThread(NULL, 0, producer, NULL, 0, NULL);
    consumer_thread = CreateThread(NULL, 0, consumer, NULL, 0, NULL);

    // Wait for both threads to finish execution
    HANDLE threads[2] = { producer_thread, consumer_thread };
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);

    // Clean up handles to prevent resource leaks
    CloseHandle(producer_thread);
    CloseHandle(consumer_thread);
    CloseHandle(mutex);
    CloseHandle(empty_slots);
    CloseHandle(full_slots);

    printf("\nExecution completed successfully without race conditions.\n");
    return 0;
}