#include "chashmap.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct find_res_t
{
    Node_HM *prev;
    Node_HM *target;
    int ret;
} find_res;

// define a node in the hashmap
typedef struct Node_HM_t
{
    long m_val; // value of the node
    // char padding[PAD];
    struct Node_HM_t *m_next; // pointer to next node in the bucket
} Node_HM;

// defining a bucket in the hashmap
typedef struct List_t
{
    Node_HM *sentinel; // list of nodes in a bucket
    pthread_mutex_t lock;
} List;

// defining the hashmap
typedef struct hm_t
{
    List **buckets; // list of buckets in the hashmap
    // pthread_mutex_t lock;
    size_t num_of_bucket;
} HM;

// iterator find helper function
// return 0 if found, return 1 if not found
find_res find_hashmap(HM *hm, long val)
{
    find_res res;
    res.prev = NULL;
    res.target = NULL;
    res.ret = 1;
    size_t bucket_idx = val % hm->num_of_bucket;
    Node_HM *cur_node = hm->buckets[bucket_idx]->sentinel;
    Node_HM *prev_node = cur_node;
    if (cur_node == NULL)
        return res;
    while (cur_node->m_val != val)
    {
        if (cur_node->m_next == NULL)
        {
            // res.prev = cur_node;
            return res;
        }
        prev_node = cur_node;
        cur_node = cur_node->m_next;
    }
    res.prev = prev_node;
    res.target = cur_node;
    res.ret = 0;
    return res;
}

// allocate a hashmap with given number of buckets
HM *alloc_hashmap(size_t n_buckets)
{
    // printf("allocate with %ld bucket\n", n_buckets);
    HM *ret = (HM *)malloc(sizeof(HM));
    ret->num_of_bucket = n_buckets;
    // pthread_mutex_init(&ret->lock, NULL);
    ret->buckets = (List **)malloc(sizeof(List *) * n_buckets);
    for (int i = 0; i < n_buckets; ++i)
    {
        ret->buckets[i] = (List *)malloc(sizeof(List));
        ret->buckets[i]->sentinel = NULL;
        pthread_mutex_init(&ret->buckets[i]->lock, NULL);
    }
    return ret;
}

// free a hashamp
void free_hashmap(HM *hm)
{
    // iterate all buckets to clean their nodes
    for (int i = 0; i < hm->num_of_bucket; ++i)
    {
        Node_HM *cur_node = hm->buckets[i]->sentinel;
        Node_HM *del_node = NULL;
        while (cur_node != NULL)
        {
            del_node = cur_node;
            cur_node = cur_node->m_next;
            free(del_node);
        }
        pthread_mutex_destroy(&hm->buckets[i]->lock);
        free(hm->buckets[i]);
    }
    free(hm->buckets);
    // pthread_mutex_destroy(&hm->lock);
    free(hm);
}

// insert val into the hm and return 0 if successful
// return 1 otherwise, e.g., could not allocate memory
int insert_item(HM *hm, long val)
{
    Node_HM *new_node = (Node_HM *)malloc(sizeof(Node_HM));
    new_node->m_val = val;
    size_t bucket_idx = val % hm->num_of_bucket;

    // pthread_mutex_lock(&hm->lock);
    pthread_mutex_lock(&hm->buckets[bucket_idx]->lock);

    // printf("insert with %ld\n", val);
    find_res res = find_hashmap(hm, val);
    // val already exist
    if (res.ret == 0)
    {
        pthread_mutex_unlock(&hm->buckets[bucket_idx]->lock);
        free(new_node);
        // pthread_mutex_unlock(&hm->lock);
        return 1;
    }

    new_node->m_next = hm->buckets[bucket_idx]->sentinel;
    hm->buckets[bucket_idx]->sentinel = new_node;

    // pthread_mutex_unlock(&hm->lock);
    pthread_mutex_unlock(&hm->buckets[bucket_idx]->lock);
    return 0;

    return 1;
}

// remove val from the hm, if it exist and return 0 if successful
// return 1 if item is not found
int remove_item(HM *hm, long val)
{
    // pthread_mutex_lock(&hm->lock);
    size_t bucket_idx = val % hm->num_of_bucket;
    pthread_mutex_lock(&hm->buckets[bucket_idx]->lock);
    // printf("remove with %ld\n", val);
    find_res res = find_hashmap(hm, val);
    // val not exist
    if (res.ret == 1)
        return 1;
    if (res.prev != NULL)
        res.prev->m_next = res.target->m_next;
    else
        free(res.target);
    // pthread_mutex_unlock(&hm->lock);
    pthread_mutex_unlock(&hm->buckets[bucket_idx]->lock);
    return 0;

    return 1;
}

// check if val exists in hm, return 0 if found, return 1 otherwise
int lookup_item(HM *hm, long val)
{
    size_t bucket_idx = val % hm->num_of_bucket;
    pthread_mutex_lock(&hm->buckets[bucket_idx]->lock);
    // pthread_mutex_lock(&hm->lock);
    // printf("lookup with %ld\n", val);
    find_res res = find_hashmap(hm, val);
    // pthread_mutex_unlock(&hm->lock);
    pthread_mutex_unlock(&hm->buckets[bucket_idx]->lock);
    return res.ret;
}

void print_hashmap(HM *hm)
{
    for (int i = 0; i < hm->num_of_bucket; ++i)
    {
        printf("Bucket %d", i + 1);
        Node_HM *cur_node = hm->buckets[i]->sentinel;
        while (cur_node != NULL)
        {
            printf(" - %ld", cur_node->m_val);
            cur_node = cur_node->m_next;
        }
        printf("\n");
    }
}
