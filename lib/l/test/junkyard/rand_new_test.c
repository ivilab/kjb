#include "l/l_sys_rand.h"
#include "l_mt/l_sys_rand_mt.h"
#include <stdio.h>
#include <pthread.h>

#define SEED 12345
#define NUM_THREADS 4
#define NUM_ITERS 10

void *thread_task(void *param) {
    mt_rand_seed(pthread_self(), SEED);

    int i = NUM_ITERS;
    while(i-- > 0) {
        printf("  %x: %f\n", pthread_self(), kjb_rand());
        int j = 1000;
        while(j--);
    }
}

void non_threaded() {
    printf("Non-threaded tests...\n");
    kjb_seed_rand(0, SEED);
    
    int i = NUM_ITERS;
    while(i-- > 0) {
        printf("%f\n", kjb_rand());
    }
}

void threaded() {
    printf("Threaded tests...\n");
    
    int n = NUM_THREADS;
    
    pthread_t threads[n];
    
    /* Initialization is required in order to use mt_rand() */
    mt_rand_init(n);
    
    /* Programmer has to set the function pointer */
    kjb_set_rand_function(mt_rand);
    
    int t;
    for(t=0; t<n; t++) {
        pthread_create(&threads[t], NULL, thread_task, (void *)&t);
    }
    for(t=0; t<n; t++) {
        pthread_join(threads[t], NULL);
    }
    
    mt_rand_clean();
}

int main() {
    non_threaded();
    threaded();
    
    return 0;   
}
