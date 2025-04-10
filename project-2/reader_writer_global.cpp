#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
using namespace std;

volatile bool lock = false;
int readCount = 0;
volatile bool done = false;

pthread_mutex_t printLock = PTHREAD_MUTEX_INITIALIZER;

void acquire() {
    while (__sync_lock_test_and_set(&lock, true));
}

void release() {
    __sync_lock_release(&lock);
}

void* reader(void* arg) {
    int id = *((int*)arg);
    delete (int*)arg;

    pthread_mutex_lock(&printLock);
    cout << "[DEBUG] Reader thread (" << id << ") started.\n";
    pthread_mutex_unlock(&printLock);

    while (!done) {
        usleep(rand() % 1001 * 1000);

        acquire();
        readCount++;
        release();

        pthread_mutex_lock(&printLock);
        cout << "File is read by reader thread (" << id << ")\n";
        pthread_mutex_unlock(&printLock);

        usleep(rand() % 10001 * 1000);

        acquire();
        readCount--;
        release();
    }

    return nullptr;
}

void* writer(void* arg) {
    int id = *((int*)arg);
    delete (int*)arg;

    pthread_mutex_lock(&printLock);
    cout << "[DEBUG] Writer thread (" << id << ") started.\n";
    pthread_mutex_unlock(&printLock);

    while (!done) {
        usleep(rand() % 1001 * 1000);

        acquire();
        pthread_mutex_lock(&printLock);
        cout << "File is written by writer thread (" << id << ")\n";
        pthread_mutex_unlock(&printLock);
        usleep(rand() % 10001 * 1000);
        release();
    }

    return nullptr;
}

int main() {
    srand(time(nullptr));

    int runtimeSeconds, numReaders, numWriters;
    cout << "Enter runtime (sec), number of readers, number of writers: ";
    cin >> runtimeSeconds >> numReaders >> numWriters;

    pthread_t* readers = new pthread_t[numReaders];
    pthread_t* writers = new pthread_t[numWriters];

    for (int i = 0; i < numReaders; ++i) {
        int* id = new int(i + 1);
        pthread_create(&readers[i], nullptr, reader, id);
    }

    for (int i = 0; i < numWriters; ++i) {
        int* id = new int(i + 1);
        pthread_create(&writers[i], nullptr, writer, id);
    }

    sleep(runtimeSeconds);
    done = true;
    usleep(500000); // allow threads to finish printing

    pthread_mutex_lock(&printLock);
    cout << "Time's up! Simulation ending.\n";
    pthread_mutex_unlock(&printLock);

    for (int i = 0; i < numReaders; i++) pthread_join(readers[i], nullptr);
    for (int i = 0; i < numWriters; i++) pthread_join(writers[i], nullptr);

    delete[] readers;
    delete[] writers;

    return 0;
}

