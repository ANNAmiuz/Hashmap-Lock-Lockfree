#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "chashmap.h"
#include "timer.h"

/* Items inserted / removed from the hashmap */
const int NUM_ITEMS = 100000;

/* Lookups done per benchmark */
const int NUM_LOOKUPS = 10000;

/* Quick validation of the hashmap */
void validate(HM *hm) {
    int num = rand();

    if (lookup_item(hm, num) == 0) {
        fprintf(stderr, "Hashmap found item %d when it was not present\n", num);
        exit(1);
    }

    if (insert_item(hm, num) != 0) {
        fprintf(stderr, "Hashmap did not insert item %d\n", num);
        exit(1);
    }

    if (lookup_item(hm, num) != 0) {
        fprintf(stderr, "Hashmap did not find item %d when it was present\n", num);
        exit(1);
    }

    if (remove_item(hm, num) != 0) {
        fprintf(stderr, "Hashmap did not remove item %d\n", num);
        exit(1);
    }
}

/* Fill the hashmap with NUM_ITEMS */
void fill(HM *hm, int *items) {
    for (int i = 0; i < NUM_ITEMS; i++) {
        int num = rand();

        if (insert_item(hm, num) != 0) {
            fprintf(stderr, "Hashmap did not insert item %d\n", num);
            exit(1);
        }

        items[i] = num;
    }
}

/* Clear the hashmap */
void clear(HM *hm, int *items) {
    for (int i = 0; i < NUM_ITEMS; i++) {
        int num = items[i];
        if (remove_item(hm, num) != 0) {
            fprintf(stderr, "Hashmap did not remove item %d\n", num);
            exit(1);
        }
    }
}

/* Benchmark the hashmap with lookups */
void benchmark(HM *hm) {
    TIMER_T start, end;

    TIMER_READ(start);

    for (int i = 0; i < NUM_LOOKUPS; i++) {
        int num = rand();
        lookup_item(hm, num);
    }

    TIMER_READ(end);

    printf("%lf\n", TIMER_DIFF_SECONDS(start, end));
}

int main(int argc, char **argv) {
    srand((int)time(NULL));

    HM *hm = alloc_hashmap(1);

    if (!hm) {
        fprintf(stderr, "Hashmap could not be allocated\n");
        exit(1);
    }

    validate(hm);

    /* First run */
    int *items = calloc(NUM_ITEMS, sizeof(int));
    fill(hm, items);
    benchmark(hm);
    
    clear(hm, items);
    free(items);

    /* Second run */
    items = calloc(NUM_ITEMS, sizeof(int));
    fill(hm, items);
    benchmark(hm);

    free_hashmap(hm);
    free(items);
}