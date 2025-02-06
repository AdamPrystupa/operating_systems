#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void* thread_function(void* arg) {
    int thread_id = *(int*)arg;
    double wynik = 0;

    for (int i = 1; i < 1000; ++i) {
        wynik += log(i);
    }
    printf("Hello OpSys. Written by thread ID: %d\n", thread_id);
    pthread_exit(NULL);
}

int main() {
    int num_threads = 5;
    pthread_t threads[num_threads];
    int thread_ids[num_threads];

    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i + 1;
        int ret = pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]);
        if (ret) {
            printf("ERROR: niepowodzenie przy tworzeniu watku %d\n", i);
            exit(-1);
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
