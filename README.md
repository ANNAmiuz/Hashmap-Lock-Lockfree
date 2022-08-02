# Concurrent Programming

We implement a simple locking construct by relying only on primitive atomic operations, and also implement a scalable, thread-safe concurrent data-structure. This will be achieved by following two different paths:
1. using locks
2. using a lock-free algorithm.

## Deliverables

1. Implement the spinlock that is declared in `cspinlock.h` to generate a library called `libcspinlock.so`. 
2. Implement a lock-based hashmap datastructure that is declared in `chashmap.h` to generate a library called `liblockhashmap.so`.
3. Implement a lock-free hashmap datastructure that is declared in `chashmap.h` to generate a library called `liblockfreehashmap.so`. 

**NB:** For tasks 1 and 3, only atomic primitives (e.g., compare-and-swap, atomic fetch-and-increment, etc.) are allowed. No synchronization library is used.

## Test Setup
There are three tests that needs to be passed to complete this task successfully.

### Mutual exclusion
`test_mutual_exclusion.py` expects `libcspinlock.so` and checks whether the implementation of `cspinlock` guarantees that no two critical sections can execute concurrently.

### Lock-based Hashmap
`test_lockhashmap.py` expects `liblockhashmap.so` and checks for the following:
1. the concurrent hashmap is implemented correctly
2. the hashmap scales with increasing number of threads

### Lock-free Hashmap
`test_lockfreehashmap.py` expects `liblockfreehashmap.so` and checks for the following:
1. the concurrent hashmap is implemented correctly
2. the hashmap scales with increasing number of threads in high contention workloads.


## References:
1. [MCS locks and qspinlocks](https://lwn.net/Articles/590243/)
2. [A Pragmatic Implementation of Non-Blocking Linked-Lists](https://www.cl.cam.ac.uk/research/srg/netos/papers/2001-caslists.pdf)
3. [Introduction to RCU](http://www2.rdrop.com/~paulmck/RCU/)
4. [Reclaiming Memory for Lock-Free Data Structures: There has to be a Better Way](https://arxiv.org/pdf/1712.01044.pdf)


 
