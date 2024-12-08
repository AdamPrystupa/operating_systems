#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

bool is_prime(long num) {
    if (num < 2) return false;
    for (long i = 2; i * i <= num; i++) {
        if (num % i == 0) return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("ERROR: niepoprawne wywolanie\n");
        return 1;
    }

    long N = atol(argv[1]);
    long M = atol(argv[2]);

    if (N <= 0 || M <= 0) {
        printf("ERROR: N i M musza byc dodatnimi liczbami\n");
        return 1;
    }

    long sum = 0;      
    long count = 0;    
    long current = M;  

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    while (count < N) {
        if (is_prime(current)) {
            sum += current;
            count++;    
        }
        current++;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed_time = (end.tv_sec - start.tv_sec) + 
                          (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Suma pierwszych %ld liczb pierwszych zaczynajac od %ld to: %ld\n", N, M, sum);
    printf("Czas wykonania: %.6f sekund\n", elapsed_time);

    return 0;
}
