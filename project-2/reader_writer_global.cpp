#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
using namespace std;

// Global spinlock and shared state
volatile bool lock = false;
int readCount = 0;
pthread_mutex_t printLock = PTHREAD_MUTEX_INITIALIZER;

// Helper spinlock functions
void acquire()
{
    while (__sync_lock_test_and_set(&lock, true))
    {
        // Busy-wait
    }
}
void release()
{
    __sync_lock_release(&lock);
}

// Reader thread
void *reader(void *arg)
{
    int id = *((int *)arg);
    delete (int *)arg;

    pthread_mutex_lock(&printLock);
    cout << "[DEBUG] Reader thread (" << id << ") started.\n";
    pthread_mutex_unlock(&printLock);

    while (true)
    {
        usleep(rand() % 1001 * 1000); // wait before trying to read

        acquire();
        readCount++;
        release();

        pthread_mutex_lock(&printLock);
        cout << "File is read by reader thread (" << id << ")\n";
        pthread_mutex_unlock(&printLock);

        usleep(rand() % 10001 * 1000); // simulate reading

        acquire();
        readCount--;
        release();
    }

    return nullptr;
}

// Writer thread
void *writer(void *arg)
{
    int id = *((int *)arg);
    delete (int *)arg;

    pthread_mutex_lock(&printLock);
    cout << "[DEBUG] Writer thread (" << id << ") started.\n";
    pthread_mutex_unlock(&printLock);

    while (true)
    {
        usleep(rand() % 1001 * 1000); // wait before trying to write

        acquire();

        pthread_mutex_lock(&printLock);
        cout << "File is written by writer thread (" << id << ")\n";
        pthread_mutex_unlock(&printLock);

        usleep(rand() % 10001 * 1000); // simulate writing
        release();
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
        cerr << "Error: Readers and Writers must be between 1 and 9.\n";
        return 1;
    }

    pthread_t *readers = new pthread_t[numReaders];
    pthread_t *writers = new pthread_t[numWriters];

    for (int i = 0; i < numReaders; i++)
    {
        int *id = new int(i + 1);
        pthread_create(&readers[i], nullptr, reader, id);
    }

    for (int i = 0; i < numWriters; i++)
    {
        int *id = new int(i + 1);
        pthread_create(&writers[i], nullptr, writer, id);
    }

    sleep(runtimeSeconds);
    pthread_mutex_lock(&printLock);
    cout << "Time's up! Simulation ending.\n";
    pthread_mutex_unlock(&printLock);

    for (int i = 0; i < numReaders; i++)
        pthread_cancel(readers[i]);
    for (int i = 0; i < numWriters; i++)
        pthread_cancel(writers[i]);
    for (int i = 0; i < numReaders; i++)
        pthread_join(readers[i], nullptr);
    for (int i = 0; i < numWriters; i++)
        pthread_join(writers[i], nullptr);

    delete[] readers;
    delete[] writers;

    return 0;
}
