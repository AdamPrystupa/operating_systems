#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Zmienne globalne
int N;                // Liczba liczb pierwszych do znalezienia
int M = 2;            // Początkowa liczba do sprawdzania (domyślnie 2)
int K = 1;            // Liczba wątków obliczeniowych (domyślnie 1)
int sprawdz;          // Następna liczba do sprawdzania
int znalezione = 0;   // Liczba znalezionych liczb pierwszych
long suma = 0;        // Suma liczb pierwszych

bool is_prime(int num) {
    if (num < 2) return false;
    for (int i = 2; i * i <= num; i++) {
        if (num % i == 0) return false;
    }
    return true;
}

void *find_primes(void *arg) {
    while (znalezione < N) {
        int current = sprawdz++; 
        if (is_prime(current)) {
            suma += current;  
            znalezione++;     
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("ERROR: niepoprawne wywolanie\n");
        return 1;
    }
    N = atoi(argv[1]);
    if (argc > 2) M = atoi(argv[2]);
    if (argc > 3) K = atoi(argv[3]);

    sprawdz = M; 


    pthread_t threads[K];
    for (int i = 0; i < K; i++) {
        if (pthread_create(&threads[i], NULL, find_primes, NULL)) {
            printf("ERROR: niepowodzenie przy tworzeniu watku %d\n", i);
            return 1;
        }
    }

    for (int i = 0; i < K; i++) {
        pthread_join(threads[i], NULL);
    }


    printf("Suma pierwszych %d liczb pierwszych zaczynajac od %d to: %ld\n", N, M, suma);

    return 0;
}

