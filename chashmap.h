#include <stddef.h>

#define PAD 64
#ifdef cppext
extern "C"
{
#endif

	/*
	Hashmap: list of buckets
	bucket1 -> sentinel -> node1 -> node2 -> NULL
	bucket2 -> sentinel -> node3 -> NULL
	...
	bucketN -> sentinel -> NULL
	*/

	// define a node in the hashmap
	// typedef struct Node_HM_t
	// {
	// 	long m_val; // value of the node
	// 	char padding[PAD];
	// 	struct Node_HM_t *m_next; // pointer to next node in the bucket
	// } Node_HM;

	struct Node_HM_t;
	typedef struct Node_HM_t Node_HM;

	// defining a bucket in the hashmap
	// typedef struct List_t
	// {
	// 	Node_HM *sentinel; // list of nodes in a bucket
	// } List;

	struct List_t;
	typedef struct List_t List;

	// defining the hashmap
	// typedef struct hm_t
	// {
	// 	List **buckets; // list of buckets in the hashmap
	// } HM;

	struct hm_t;
	typedef struct hm_t HM;

	// allocate a hashmap with given number of buckets
	HM *alloc_hashmap(size_t n_buckets);

	// free a hashamp
	void free_hashmap(HM *hm);

	// insert val into the hm and return 0 if successful
	// return 1 otherwise, e.g., could not allocate memory
	int insert_item(HM *hm, long val);

	// remove val from the hm, if it exist and return 0 if successful
	// return 1 if item is not found
	int remove_item(HM *hm, long val);

	// check if val exists in hm, return 0 if found, return 1 otherwise
	int lookup_item(HM *hm, long val);

	// search helper function
	Node_HM *search(HM *hm, size_t bucket_idx, long val, Node_HM **left_node);

	// print all elements in the hashmap as follows:
	// Bucket 1 - val1 - val2 - val3 ...
	// Bucket 2 - val4 - val5 - val6 ...
	// Bucket N -  ...
	void print_hashmap(HM *hm);
#ifdef cppext
}
#endif