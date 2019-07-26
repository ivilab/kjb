#include "l_mt/l_sys_rand_mt.h"
#include <time.h>
#include <pthread.h>

#define NUM_ITERS 100000
#define NUM_THREADS 32
#define SEED 0x100000002

#warning "[Code police] Pthreads references need guards."

void *thread_task(void *param) {
    /*printf("thread_task, pthread_self() = %x\n", pthread_self());*/
    
    /*mt_rand_seed(pthread_self(), *((int*)param));*/
    mt_rand_seed(pthread_self(), SEED);
    
    struct timespec ts = {0, 500000000};
    
    int i = NUM_ITERS;
    while(i-- > 0) {
        printf("  %x: %f\n", pthread_self(), mt_rand());
        int j = 1000;
        while(j--);
    }
}

int main() {
    int n = NUM_THREADS;
    
    pthread_t threads[n];
    
    mt_rand_init(n);
    
    int t;
    for(t=0; t<n; t++) {
        pthread_create(&threads[t], NULL, thread_task, (void *)&t);
    }
    for(t=0; t<n; t++) {
        pthread_join(threads[t], NULL);
    }
    
    mt_rand_clean();

    return 0;
}
