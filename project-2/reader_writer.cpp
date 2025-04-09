#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

using namespace std;

// Maximums
const int MAX_READERS = 9;
const int MAX_WRITERS = 3;

// Global state
int readCount = 0;
sem_t mutex;    // to protect readCount
sem_t writeSem; // to block writers or readers when a writer is writing

// Simulation duration
int runtimeSeconds;

// Reader thread function
void *reader(void *arg)
{
    int id = *((int *)arg);
    while (true)
    {
        usleep(rand() % 1001 * 1000); // wait before accessing (0-1000ms)

        // Entry section
        sem_wait(&mutex);
        readCount++;
        if (readCount == 1)
        {
            sem_wait(&writeSem); // first reader locks writers out
        }
        sem_post(&mutex);

        // Critical section
        cout << "File is read by reader thread (" << id << ")" << endl;
        usleep(rand() % 10001 * 1000); // access file (0-10000ms)

        // Exit section
        sem_wait(&mutex);
        readCount--;
        if (readCount == 0)
        {
            sem_post(&writeSem); // last reader allows writers back in
        }
        sem_post(&mutex);
    }
    return nullptr;
}

// Writer thread function
void *writer(void *arg)
{
    int id = *((int *)arg);
    while (true)
    {
        usleep(rand() % 1001 * 1000); // wait before accessing (0-1000ms)

        sem_wait(&writeSem); // exclusive access
        cout << "File is written by writer thread (" << id << ")" << endl;
        usleep(rand() % 10001 * 1000); // access file (0-10000ms)
        sem_post(&writeSem);
    }
    return nullptr;
}

int main()
{
    srand(time(nullptr));

    // Input: runtime, number of readers, number of writers
    int numReaders, numWriters;
    cout << "Enter runtime (sec), number of readers, number of writers: ";
    cin >> runtimeSeconds >> numReaders >> numWriters;

    // Init semaphores
    sem_init(&mutex, 0, 1);
    sem_init(&writeSem, 0, 1);

    // Create threads
    pthread_t readers[MAX_READERS], writers[MAX_WRITERS];
    int ids[MAX_READERS > MAX_WRITERS ? MAX_READERS : MAX_WRITERS];

    for (int i = 0; i < max(numReaders, numWriters); i++)
    {
        ids[i] = i + 1;
    }

    for (int i = 0; i < numReaders; i++)
    {
        pthread_create(&readers[i], nullptr, reader, &ids[i]);
    }
    for (int i = 0; i < numWriters; i++)
    {
        pthread_create(&writers[i], nullptr, writer, &ids[i]);
    }

    sleep(runtimeSeconds); // Run simulation

    cout << "Time's up! Simulation ending.\n";
    exit(0); // End simulation
}
