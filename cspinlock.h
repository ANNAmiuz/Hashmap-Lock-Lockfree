#ifdef cppext
extern "C"{
#endif
typedef struct cspinlock cspinlock_t;

//acquire the lock
extern int cspin_lock(cspinlock_t *slock);

//if the lock can not be acquired, return immediately
extern int cspin_trylock(cspinlock_t *slock);

//release the lock
extern int cspin_unlock(cspinlock_t *slock);

//allocate a lock
extern cspinlock_t* cspin_alloc();

//free a lock
extern void cspin_free(cspinlock_t* slock);
#ifdef cppext
}
#endif