#include "cspinlock.h"
#include <stdlib.h>
#include "atomic_helper.h"

// #include <atomic>

// extern "C"
// {
//     struct cspinlock
//     {
//         std::atomic<bool> flag;
//     };

//     typedef struct cspinlock cspinlock_t;

//     int cspin_lock(cspinlock_t *slock)
//     {
//         bool expect = false;
//         while (!slock->flag.compare_exchange_weak(expect, true))
//         {
//             expect = false;
//         };
//         return 0;
//     }

//     int cspin_trylock(cspinlock_t *slock)
//     {
//         return !slock->flag.load(std::memory_order_relaxed) && !slock->flag.exchange(true, std::memory_order_acquire);
//     }

//     int cspin_unlock(cspinlock_t *slock)
//     {
//         slock->flag.store(0, std::memory_order_release);
//         return 0;
//     }

//     cspinlock_t *cspin_alloc()
//     {
//         cspinlock_t *ret;
//         ret = (cspinlock_t *)malloc(sizeof(cspinlock_t));
//         ret->flag = false;
//         return ret;
//     }

//     void cspin_free(cspinlock_t *slock)
//     {
//         free(slock);
//     }
// }

struct cspinlock
{
    volatile uint8_t flag;
};

typedef struct cspinlock cspinlock_t;

int cspin_lock(cspinlock_t *slock)
{
    // int expect = 0;
    while (CAS_U8(&slock->flag, 0, 1))
    {
        // expect = 0;
    };
    return 0;
}

int cspin_trylock(cspinlock_t *slock)
{
    // return !slock->flag.load(std::memory_order_relaxed) && !slock->flag.exchange(true, std::memory_order_acquire);
    return CAS_U8(&slock->flag, 0, 1);
}

int cspin_unlock(cspinlock_t *slock)
{
    // slock->flag.store(0, std::memory_order_release);
    CAS_U8(&slock->flag, 1, 0);
    return 0;
}

cspinlock_t *cspin_alloc()
{
    cspinlock_t *ret;
    ret = (cspinlock_t *)malloc(sizeof(cspinlock_t));
    ret->flag = 0;
    return ret;
}

void cspin_free(cspinlock_t *slock)
{
    free(slock);
}
