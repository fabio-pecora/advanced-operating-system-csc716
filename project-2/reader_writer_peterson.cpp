#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
using namespace std;

volatile bool flag[2] = {false, false};
volatile int turn = 0;
volatile bool done = false;

pthread_mutex_t printLock = PTHREAD_MUTEX_INITIALIZER;

void peterson_lock(int self) {
    int other = 1 - self;
    flag[self] = true;
    turn = other;
    while (flag[other] && turn == other);
}

void peterson_unlock(int self) {
    flag[self] = false;
}

void* reader(void* arg) {
    int id = *((int*)arg);
    delete (int*)arg;

    pthread_mutex_lock(&printLock);
    cout << "[DEBUG] Reader thread (" << id << ") started.\n";
    pthread_mutex_unlock(&printLock);

    while (!done) {
        usleep(rand() % 1001 * 1000);

        peterson_lock(0);
        pthread_mutex_lock(&printLock);
        cout << "File is read by reader thread (" << id << ")\n";
        pthread_mutex_unlock(&printLock);
        usleep(rand() % 10001 * 1000);
        peterson_unlock(0);
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

        peterson_lock(1);
        pthread_mutex_lock(&printLock);
        cout << "File is written by writer thread (" << id << ")\n";
        pthread_mutex_unlock(&printLock);
        usleep(rand() % 10001 * 1000);
        peterson_unlock(1);
    }

    return nullptr;
}

int main() {
    srand(time(nullptr));
    int runtimeSeconds;
    cout << "Enter runtime (sec): ";
    cin >> runtimeSeconds;

    pthread_t readerThread, writerThread;

    int* rID = new int(1);
    int* wID = new int(1);

    pthread_create(&readerThread, nullptr, reader, rID);
    pthread_create(&writerThread, nullptr, writer, wID);

    sleep(runtimeSeconds);
    done = true;
    usleep(500000); // allow final prints

    pthread_mutex_lock(&printLock);
    cout << "Time's up! Simulation ending.\n";
    pthread_mutex_unlock(&printLock);

    pthread_join(readerThread, nullptr);
    pthread_join(writerThread, nullptr);

    return 0;
}
