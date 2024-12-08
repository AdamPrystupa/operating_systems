#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


typedef struct {
    int X, Y;                  // Rozmiar tablicy
    int N;                     // Liczba synow              
    int **territory;           // Tablica terenow
    int *captures;             // Zdobyte tereny dla kazdego syna
    int sons_finished;         // Licznik zakonczonych watkow synow
    pthread_mutex_t mutex;     // Mutex do synchronizacji
    pthread_cond_t condition;  // Zmienna warunkowa
} SharedData;


typedef struct {
    SharedData *shared_data;
    int son_id;
    int S;
} SonData;


void *son(void *arg) {
    SonData *son_data = (SonData *)arg;
    SharedData *shared_data = son_data->shared_data;
    int son_id = son_data->son_id;
    int S = son_data->S;

    free(son_data);

    for (int i = 0; i < S; i++) {
        int x = (int)(drand48() * shared_data->X);
        int y = (int)(drand48() * shared_data->Y);

        pthread_mutex_lock(&shared_data->mutex);
        if (shared_data->territory[x][y] == -1) {  
            shared_data->territory[x][y] = son_id;
            shared_data->captures[son_id]++;
            printf("Syn %d zajmuje teren (%d, %d).\n", son_id, x, y);
        } else {
            printf("Syn %d nie zajmuje terenu (%d, %d)\n", son_id, x, y);
        }
        pthread_mutex_unlock(&shared_data->mutex);
        usleep(1000); 
    }

    pthread_mutex_lock(&shared_data->mutex);
    shared_data->sons_finished++;
    pthread_cond_signal(&shared_data->condition);
    pthread_mutex_unlock(&shared_data->mutex);

    return NULL;
}



void *rejent(void *arg) {
    SharedData *shared_data = (SharedData *)arg;

    pthread_mutex_lock(&shared_data->mutex);
    while (shared_data->sons_finished < shared_data->N) {
        pthread_cond_wait(&shared_data->condition, &shared_data->mutex);
    }

    printf("\nPodsumowanie zdobytych terenow:\n");
    for (int i = 0; i < shared_data->N; i++) {
        printf("Syn %d zdobył %d terenow.\n", i, shared_data->captures[i]);
    }
    printf("\nSzczegoły terenow:\n");

    int unclaimed = 0;
        for (int j = 0; j < shared_data->Y; j++) {
    for (int i = 0; i < shared_data->X; i++) {
            if (shared_data->territory[i][j] == -1) {
                unclaimed++;
            }
            printf("[%d]",shared_data->territory[i][j]);
        }
        printf("\n");
    }
    printf("\nNiezajete tereny: %d\n", unclaimed);

    pthread_mutex_unlock(&shared_data->mutex);
    return NULL;
}

int main() {
    int N, X, Y, S;

    printf("Podaj liczbe synow (N): ");
    scanf("%d", &N);
    printf("Podaj liczbe kolumn terytorium (X): ");
    scanf("%d", &X);
    printf("Podaj liczbe wierszy terytorium (Y): ");
    scanf("%d", &Y);
    printf("Podaj liczbe szans dla kazdego syna (S): ");
    scanf("%d", &S);

    srand48(time(NULL) ^ getpid()); 

    SharedData shared_data;
    shared_data.X = X;
    shared_data.Y = Y;
    shared_data.N =N ;    
    shared_data.sons_finished = 0;
    shared_data.territory = malloc(X * sizeof(int *));
    shared_data.captures = calloc(N, sizeof(int));
    for (int i = 0; i < X; i++) {
        shared_data.territory[i] = malloc(Y * sizeof(int));
        for (int j = 0; j < Y; j++) {
            shared_data.territory[i][j] = -1; 
        }
    }

    pthread_mutex_init(&shared_data.mutex, NULL);
    pthread_cond_init(&shared_data.condition, NULL);

    pthread_t sons[N];
    pthread_t rejent_thread;
    pthread_create(&rejent_thread, NULL, rejent, &shared_data);

    for (int i = 0; i < N; i++) {
        SonData *son_data = malloc(sizeof(SonData));
        son_data->shared_data = &shared_data;
        son_data->son_id = i;
        son_data->S = S;

        pthread_create(&sons[i], NULL, son, son_data);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(sons[i], NULL);
    }
    pthread_join(rejent_thread, NULL);

    for (int i = 0; i < X; i++) {
        free(shared_data.territory[i]);
    }
    free(shared_data.territory);
    free(shared_data.captures);
    pthread_mutex_destroy(&shared_data.mutex);
    pthread_cond_destroy(&shared_data.condition);

    return 0;
}