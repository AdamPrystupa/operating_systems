#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>

// Zmienne globalne
int N;                // Liczba liczb pierwszych do znalezienia
int M = 2;            // Początkowa liczba do sprawdzania (domyślnie 2)
int K = 1;            // Liczba wątków obliczeniowych (domyślnie 1)
int sprawdz;          // Następna liczba do sprawdzania
int znalezione = 0;   // Liczba znalezionych liczb pierwszych
long suma = 0;        // Suma liczb pierwszych

pthread_mutex_t muteks_pobierania;
pthread_mutex_t muteks_sumowania;

bool is_prime(int num) {
    if (num < 2) return false;
    for (int i = 2; i * i <= num; i++) {
        if (num % i == 0) return false;
    }
    return true;
}

void *find_primes(void *arg) {
    while (1) {
        int current;

        pthread_mutex_lock(&muteks_pobierania);
        if (znalezione >= N) {
            pthread_mutex_unlock(&muteks_pobierania);
            break;
        }
        current = sprawdz++;
        pthread_mutex_unlock(&muteks_pobierania);

        if (is_prime(current)) {
            pthread_mutex_lock(&muteks_sumowania);
            if (znalezione < N) {
                suma += current;
                znalezione++;
            }
            pthread_mutex_unlock(&muteks_sumowania);
        }
    }
    pthread_exit(NULL);
}

long get_time_in_microseconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000L + tv.tv_usec;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("ERROR: niepoprawne wywolanie", argv[0]);
        return 1;
    }
    N = atoi(argv[1]);
    if (argc > 2) M = atoi(argv[2]);
    if (argc > 3) K = atoi(argv[3]);

    sprawdz = M;

    pthread_mutex_init(&muteks_pobierania, NULL);
    pthread_mutex_init(&muteks_sumowania, NULL);

    long start_time = get_time_in_microseconds();

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

    long end_time = get_time_in_microseconds();

    printf("Suma pierwszych %d liczb pierwszych zaczynając od %d to: %ld\n", N, M, suma);
    printf("Czas działania programu: %.3f sekund\n", (end_time - start_time) / 1e6);

    pthread_mutex_destroy(&muteks_pobierania);
    pthread_mutex_destroy(&muteks_sumowania);

    return 0;
}