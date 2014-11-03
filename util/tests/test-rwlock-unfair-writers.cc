// check if write locks are fair

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <portability/toku_pthread.h>

pthread_rwlock_t rwlock;
volatile int killed = 0;

static void *t1_func(void *arg) {
    int i;
    for (i = 0; !killed; i++) {
        int r;
        r = toku_pthread_rwlock_wrlock(&rwlock); 
        assert(r == 0);
        usleep(10000);
        r = toku_pthread_rwlock_unlock(&rwlock);
        assert(r == 0);
    }
    printf("%lu %d\n", (unsigned long) toku_pthread_self(), i);
    return arg;
}

int main(void) {
    int r;
#if 0
    rwlock = PTHREAD_RWLOCK_INITIALIZER;
#endif
#if 0
    toku_pthread_rwlockattr_t attr;
    toku_pthread_rwlockattr_init(&attr);
    toku_pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
    r = toku_pthread_rwlock_init(&rwlock, &attr);
#endif
#if 0
    toku_pthread_rwlockattr_t attr;
    toku_pthread_rwlockattr_init(&attr);
    r = toku_pthread_rwlock_init(&rwlock, &attr);
#endif
#if 1
    r = toku_pthread_rwlock_init(&rwlock, NULL);
    assert(r == 0);
#endif
    
    const int nthreads = 2;
    toku_pthread_t tids[nthreads];
    for (int i = 0; i < nthreads; i++) {
        r = toku_pthread_create(&tids[i], NULL, t1_func, NULL); 
        assert(r == 0);
    }
    sleep(10);
    killed = 1;
    for (int i = 0; i < nthreads; i++) {
        void *ret;
        r = toku_pthread_join(tids[i], &ret);
        assert(r == 0);
    }
    return 0;
}
