#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <vector>

using namespace std;

// Shared state
int readCount = 0;
sem_t mutex;
sem_t writeSem;

// Track all IDs for cleanup
vector<int *> thread_ids;

// Reader function
void *reader(void *arg)
{
    int id = *((int *)arg);
    cout << "[DEBUG] Reader thread (" << id << ") started." << endl;

    while (true)
    {
        usleep(rand() % 1001 * 1000);

        sem_wait(&mutex);
        readCount++;
        if (readCount == 1)
        {
            sem_wait(&writeSem);
        }
        sem_post(&mutex);

        cout << "File is read by reader thread (" << id << ")" << endl;
        usleep(rand() % 10001 * 1000);

        sem_wait(&mutex);
        readCount--;
        if (readCount == 0)
        {
            sem_post(&writeSem);
        }
        sem_post(&mutex);
    }

    return nullptr;
}

// Writer function
void *writer(void *arg)
{
    int id = *((int *)arg);
    cout << "[DEBUG] Writer thread (" << id << ") started." << endl;

    while (true)
    {
        usleep(rand() % 1001 * 1000);

        sem_wait(&writeSem);
        cout << "File is written by writer thread (" << id << ")" << endl;
        usleep(rand() % 10001 * 1000);
        sem_post(&writeSem);
    }

    return nullptr;
}

int main()
{
    srand(time(nullptr));

    int runtimeSeconds, numReaders, numWriters;
    cout << "Enter runtime (sec), number of readers, number of writers: ";
    cin >> runtimeSeconds >> numReaders >> numWriters;

    if (numReaders < 1 || numReaders > 9 || numWriters < 1 || numWriters > 9)
    {
        cerr << "Error: Readers and Writers must be between 1 and 9." << endl;
        return 1;
    }

    pthread_t *readers = new pthread_t[numReaders];
    pthread_t *writers = new pthread_t[numWriters];

    sem_init(&mutex, 0, 1);
    sem_init(&writeSem, 0, 1);

    // Create reader threads
    for (int i = 0; i < numReaders; i++)
    {
        int *id = new int(i + 1);
        thread_ids.push_back(id); // save pointer for cleanup
        if (pthread_create(&readers[i], nullptr, reader, id) != 0)
        {
            cerr << "Failed to create reader thread " << *id << endl;
        }
    }

    // Create writer threads
    for (int i = 0; i < numWriters; i++)
    {
        int *id = new int(i + 1);
        thread_ids.push_back(id); // save pointer for cleanup
        if (pthread_create(&writers[i], nullptr, writer, id) != 0)
        {
            cerr << "Failed to create writer thread " << *id << endl;
        }
    }

    // Run the simulation
    sleep(runtimeSeconds);
    cout << "Time's up! Simulation ending." << endl;

    for (int i = 0; i < numReaders; i++)
    {
        pthread_cancel(readers[i]);
    }
    for (int i = 0; i < numWriters; i++)
    {
        pthread_cancel(writers[i]);
    }

    for (int i = 0; i < numReaders; i++)
    {
        pthread_join(readers[i], nullptr);
    }
    for (int i = 0; i < numWriters; i++)
    {
        pthread_join(writers[i], nullptr);
    }

    // Clean up
    for (int *ptr : thread_ids)
    {
        delete ptr;
    }

    delete[] readers;
    delete[] writers;

    return 0;
}
