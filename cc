#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

using namespace std;

const int NUM_READERS = 5;
const int NUM_WRITERS = 3;

mutex mtx;
condition_variable reader_cv, writer_cv;
int readers_count = 0;
bool writing = false;

void reader(int id) {
    unique_lock<mutex> lock(mtx);

    // Wait if a writer is writing or waiting to write
    while (writing) {
        reader_cv.wait(lock);
    }

    readers_count++;
    lock.unlock();

    // Reading...
    cout << "Reader " << id << " is reading." << endl;

    lock.lock();
    readers_count--;

    // Signal a writer if no readers
    if (readers_count == 0) {
        writer_cv.notify_one();
    }

    lock.unlock();
}

void writer(int id) {
    unique_lock<mutex> lock(mtx);

    // Wait if a reader is reading or a writer is writing
    while (readers_count > 0 || writing) {
        writer_cv.wait(lock);
    }

    writing = true;
    lock.unlock();

    // Writing...
    cout << "Writer " << id << " is writing." << endl;

    lock.lock();
    writing = false;

    // Signal waiting readers or writers
    reader_cv.notify_all();
    writer_cv.notify_one();

    lock.unlock();
}

int main() {
    vector<thread> threads;

    // Create reader threads
    for (int i = 1; i <= NUM_READERS; ++i) {
        threads.emplace_back(reader, i);
    }

    // Create writer threads
    for (int i = 1; i <= NUM_WRITERS; ++i) {
        threads.emplace_back(writer, i);
    }

    // Join all threads
    for (auto& t : threads) {
        t.join();
    }

    return 0;
}


#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include <random>

using namespace std;

const int NUM_PHILOSOPHERS = 5;

mutex mtx;
vector<semaphore> chopsticks(NUM_PHILOSOPHERS, 1);

void philosopher(int id) {
    while (true) {
        // Thinking
        cout << "Philosopher " << id << " is thinking." << endl;
        this_thread::sleep_for(chrono::milliseconds(1000));

        // Pick up chopsticks
        mtx.lock();
        chopsticks[id].wait();
        chopsticks[(id + 1) % NUM_PHILOSOPHERS].wait();
        mtx.unlock();

        // Eating
        cout << "Philosopher " << id << " is eating." << endl;
        this_thread::sleep_for(chrono::milliseconds(1000));

        // Put down chopsticks
        chopsticks[id].signal();
        chopsticks[(id + 1) % NUM_PHILOSOPHERS].signal();
    }
}

int main() {
    vector<thread> philosophers;

    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        philosophers.emplace_back(philosopher, i);
    }

    for (auto& t : philosophers) {
        t.join();
    }

    return 0;
}
