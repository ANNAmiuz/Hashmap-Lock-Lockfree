#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cspinlock.h"

const int NUM_THREADS = 4;
const long ITERATIONS = 1000000;

volatile int go = 0;
volatile long counter;

cspinlock_t *slock;

void *work(void *arg) {
    while (!go) {
    }

    for (long i = 0; i < ITERATIONS; i++) {
        cspin_lock(slock);
        counter++;
        cspin_unlock(slock);
    }

    return NULL;
}

void test() {
    counter = 0;
    go = 0;

    pthread_t threads[NUM_THREADS];

    for (size_t i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, work, NULL);
    }

    go = 1;

    for (size_t i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    if (counter != NUM_THREADS * ITERATIONS) {
        fprintf(stderr, "Mutual exclusion not working correctly\n");
        exit(1);
    }
}

int main() {
    slock = cspin_alloc();
    if (!slock) {
        fprintf(stderr, "could not allocate memory\n");
        exit(1);
    }

    for (size_t i = 0; i < 10; i++) {
        test();
    }

    cspin_free(slock);

    return 0;
}