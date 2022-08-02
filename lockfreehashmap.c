#include "chashmap.h"
#include "atomic_helper.h" // DONT know which one to use for processor
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

// the implementation copies all the sample code in Harris algorithm (https://www.cl.cam.ac.uk/research/srg/netos/papers/2001-caslists.pdf)

// define a node in the hashmap
typedef struct Node_HM_t
{
    long data; // value of the node
    // char padding[PAD]; // make it faster
    struct Node_HM_t *next; // pointer to next node in the bucket
} Node_HM;

// defining a bucket in the hashmap
typedef struct List_t
{
    // uint32_t size;
    Node_HM *head; // head node of the list (empty)
    Node_HM *tail; // tail node of the list (empty)
} List;

// defining the hashmap
typedef struct hm_t
{
    List **buckets; // list of buckets in the hashmap
    size_t num_of_bucket;
} HM;

// a set of helper function for marked reference (8 bytes) / search defined in paper
inline int is_marked_ref(long i);
inline long unset_mark(long i);
inline long set_mark(long i);
inline long get_unmarked_ref(long i);
inline long get_marked_ref(long i);
Node_HM *new_node(long val, Node_HM *next);
Node_HM *list_search(List *bucket, long val, Node_HM **left_node);

// list method (thus each bucket as a list)
List *list_new();
int list_lookup(List *bucket, long val);
int list_add(List *bucket, long val);
int list_remove(List *bucket, long val);

// LSB = 1: logically deleted, LSB = 0: not deleted
inline int is_marked_ref(long i)
{
    return (int)(i & 0x1L);
}
inline long unset_mark(long i)
{
    i &= ~0x1L;
    return i;
}
inline long set_mark(long i)
{
    i |= 0x1L;
    return i;
}
inline long get_unmarked_ref(long i)
{
    return i & ~0x1L;
}
inline long get_marked_ref(long i)
{
    return i | 0x1L;
}

// search helper: locate and GC
Node_HM *list_search(List *bucket, long val, Node_HM **left_node)
{
    Node_HM *left_node_next, *right_node;
    left_node_next = right_node = NULL;
    while (1)
    {
        /* 1: Find left_node and right_node */
        Node_HM *t = bucket->head;
        Node_HM *t_next = bucket->head->next;
        while (is_marked_ref(t_next) || (t->data < val))
        {
            if (!is_marked_ref(t_next))
            {
                (*left_node) = t;
                left_node_next = t_next;
            }
            t = get_unmarked_ref(t_next);
            if (t == bucket->tail)
                break;
            t_next = t->next;
        }
        right_node = t;
        /* 2: Check nodes are adjacent */
        if (left_node_next == right_node)
        {
            if (!is_marked_ref(right_node->next))
                return right_node;
        }
        else
        {
            /* 3: Remove one or more marked nodes */
            if (CAS_PTR(&((*left_node)->next), left_node_next, right_node) == left_node_next)
            {
                if (!is_marked_ref(right_node->next))
                    return right_node;
            }
        }
    }
}

// node build helper
Node_HM *new_node(long val, Node_HM *next)
{
    Node_HM *node = malloc(1 * sizeof(Node_HM));
    node->data = val;
    node->next = next;
    return node;
}

// List-based operation
List *list_new()
{
    List *ret = malloc(sizeof(List));
    ret->head = new_node(INT_MIN, NULL);
    ret->tail = new_node(INT_MAX, NULL);
    ret->head->next = ret->tail;
    return ret;
}

// already in list: return 0
int list_insert(List *bucket, long val)
{
    Node_HM *new = new_node(val, NULL);
    Node_HM *right = NULL, *left = NULL;
    
    while (1)
    {
        right = list_search(bucket, val, &left);
        // accept duplicated key
        // if (right != the_list->tail && right->data == val)
        // {
        //     return 0;
        // }
        new->next = right;
        if (CAS_PTR(&(left->next), right, new) == right)
        {
            return 1;
        }
    }
}

// not in list: return 0
int list_remove(List *bucket, long val)
{
    Node_HM *right = NULL, *left = NULL, *right_node_next = NULL;
    
    while (1)
    {
        right = list_search(bucket, val, &left);
        // node not found
        if (right == bucket->tail || right->data != val) /*T1*/
        {
            return 0;
        }
        right_node_next = right->next;
        if (!is_marked_ref(right_node_next))
        {
            if (CAS_PTR(&(right->next), right_node_next, get_marked_ref(right_node_next)) == right_node_next)
            {
                // return 1;
                break;
            }
        }
    }
    if (!CAS_PTR(&(left->next), right, right_node_next))
        right = list_search(bucket, right->data, &left);
    return 1;
}

// not found: return 0
int list_lookup(List *bucket, long val)
{
    Node_HM *cursor = get_unmarked_ref(bucket->head->next);
    while (cursor != bucket->tail)
    {
        if (!is_marked_ref(cursor->next) && cursor->data >= val)
        {
            if (cursor->data == val)
                return 1;
            else
                return 0;
        }
        // always get unmarked pointer
        cursor = get_unmarked_ref(cursor->next);
    }
    return 0;
}

// allocate a hashmap with given number of buckets
HM *alloc_hashmap(size_t n_buckets)
{
    // printf("allocate with %ld bucket\n", n_buckets);
    HM *ret = (HM *)malloc(sizeof(HM));
    ret->num_of_bucket = n_buckets;
    ret->buckets = (List **)calloc(n_buckets, sizeof(List *));
    for (int i = 0; i < n_buckets; ++i)
    {
        ret->buckets[i] = list_new();
    }
    return ret;
}

// free a hashamp
void free_hashmap(HM *hm)
{
    // iterate all buckets to clean their nodes
    for (int i = 0; i < hm->num_of_bucket; ++i)
    {
        Node_HM *cur_node = hm->buckets[i]->head;
        Node_HM *del_node = NULL;
        while (cur_node != NULL)
        {
            del_node = cur_node;
            cur_node = get_unmarked_ref(cur_node->next);
            free(del_node);
        }
        free(hm->buckets[i]);
    }
    free(hm->buckets);
    free(hm);
}

// insert val into the hm and return 0 if successful
// return 1 otherwise, e.g., could not allocate memory
int insert_item(HM *hm, long val)
{
    size_t bucket_idx = val % hm->num_of_bucket;
    int ret = list_insert(hm->buckets[bucket_idx], val);
    if (ret == 0)
        return 1;
    else
        return 0;
}

// remove val from the hm, if it exist and return 0 if successful
// return 1 if item is not found (so odd)
int remove_item(HM *hm, long val)
{
    size_t bucket_idx = val % hm->num_of_bucket;
    int ret = list_remove(hm->buckets[bucket_idx], val);
    if (ret == 0)
        return 1;
    else
        return 0;
}

// check if val exists in hm, return 0 if found, return 1 otherwise (so odd)
int lookup_item(HM *hm, long val)
{
    size_t bucket_idx = val % hm->num_of_bucket;
    int ret = list_lookup(hm->buckets[bucket_idx], val);
    if (ret == 0)
        return 1;
    else
        return 0;
}

void print_hashmap(HM *hm)
{
    for (int i = 0; i < hm->num_of_bucket; ++i)
    {
        printf("Bucket %d", i + 1);
        Node_HM *cur_node = hm->buckets[i]->head->next;
        Node_HM *head = hm->buckets[i]->head;
        Node_HM *tail = hm->buckets[i]->tail;
        while (cur_node != NULL)
        {
            // notice: there is logically deleted node
            if (cur_node != head && cur_node != tail && !is_marked_ref(cur_node->next))
                printf(" - %ld", cur_node->data);
            cur_node = get_unmarked_ref(cur_node->next);
        }
        printf("\n");
    }
}

// remember to omit goto
// Node_HM * list_search(HM *hm, size_t bucket_idx, long val, Node_HM **left_node)
// {
//     Node_HM *left_node_next, *right_node;
// search_again:
//     do
//     {
//         Node_HM *t = hm->buckets[bucket_idx]->sentinel;
//         Node_HM *t_next = hm->buckets[bucket_idx]->sentinel->m_next;

//         do
//         {
//             if (!is_marked_reference(t_next))
//             {
//                 (*left_node) = t;
//                 left_node_next = t_next;
//             }
//             t = get_unmarked_reference(t_next);
//             if (t == hm->buckets[bucket_idx]->tail)
//                 break;
//             t_next = t->m_next;

//         } while (!is_marked_reference(t_next) || (t->m_val < val));

//         if (left_node_next == right_node)
//             if ((right_node != hm->buckets[bucket_idx]->tail) && !is_marked_reference(right_node->m_next))
//                 goto search_again;
//             else
//                 return right_node;

//         if (__sync_bool_compare_and_swap(&((*left_node)->m_next), left_node_next, right_node))
//             if (right_node != hm->buckets[bucket_idx]->tail && is_marked_reference(right_node->m_next))
//                 goto search_again;
//             else
//                 return right_node;
//     } while (1);
// }
