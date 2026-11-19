#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE];
int in = 0, out = 0;

HANDLE hEmpty;
HANDLE hFull;
CRITICAL_SECTION mutex;

DWORD WINAPI Producer(LPVOID lpParam) {
    int item;
    for (int i = 0; i < 10; i++) {
        item = rand() % 100;

        WaitForSingleObject(hEmpty, INFINITE); // Wait for empty slot
        EnterCriticalSection(&mutex);          // Enter critical section

        buffer[in] = item;
        printf("Produced: %d at index %d\n", item, in);
        in = (in + 1) % BUFFER_SIZE;

        LeaveCriticalSection(&mutex);          // Exit critical section
        ReleaseSemaphore(hFull, 1, NULL);      // Signal full slot

        Sleep(1000);
    }
    return 0;
}

DWORD WINAPI Consumer(LPVOID lpParam) {
    int item;
    for (int i = 0; i < 10; i++) {
        WaitForSingleObject(hFull, INFINITE);  // Wait for full slot
        EnterCriticalSection(&mutex);          // Enter critical section

        item = buffer[out];
        printf("Consumed: %d from index %d\n", item, out);
        out = (out + 1) % BUFFER_SIZE;

        LeaveCriticalSection(&mutex);          // Exit critical section
        ReleaseSemaphore(hEmpty, 1, NULL);     // Signal empty slot

        Sleep(1000);
    }
    return 0;
}

int main() {
    HANDLE hProducer, hConsumer;

    // Initialize Win32 semaphores and critical section
    hEmpty = CreateSemaphore(NULL, BUFFER_SIZE, BUFFER_SIZE, NULL);
    hFull = CreateSemaphore(NULL, 0, BUFFER_SIZE, NULL);
    InitializeCriticalSection(&mutex);

    // Create threads
    hProducer = CreateThread(NULL, 0, Producer, NULL, 0, NULL);
    hConsumer = CreateThread(NULL, 0, Consumer, NULL, 0, NULL);

    // Wait for threads to finish
    WaitForSingleObject(hProducer, INFINITE);
    WaitForSingleObject(hConsumer, INFINITE);

    // Cleanup
    CloseHandle(hProducer);
    CloseHandle(hConsumer);
    CloseHandle(hEmpty);
    CloseHandle(hFull);
    DeleteCriticalSection(&mutex);

    return 0;
}