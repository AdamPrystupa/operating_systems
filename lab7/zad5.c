#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>

#define BABBLE_NAME "/KPK"
#define BABBLE_MODE 0777
#define BABBLE_LIMIT 10
#define BABBLE_LENGTH 80
#define NICKNAME "AP"

struct babblespace {
    pthread_mutex_t babble_mutex;
    pthread_cond_t babble_cond;
    int babble_first, babble_total;
    char babbles[BABBLE_LIMIT][BABBLE_LENGTH];
};

void wyswietl_komunikaty(struct babblespace *ptr) {
    if (pthread_mutex_lock(&ptr->babble_mutex) != 0) {
        perror("Problem z pthread_mutex_lock");
        return;
    }

    printf("Wyświetlanie komunikatów w kolejności chronologicznej:\n");

    for (int i = 0; i < ptr->babble_total; i++) {
        int index = (ptr->babble_first + i) % BABBLE_LIMIT;
        printf("%s\n", ptr->babbles[index]);
    }

    if (pthread_mutex_unlock(&ptr->babble_mutex) != 0) {
        perror("Problem z pthread_mutex_unlock");
    }
}

void dodaj_komunikat(struct babblespace *ptr, const char *komunikat) {
    printf("jestem");
    if (pthread_mutex_lock(&ptr->babble_mutex) != 0) {
        perror("Problem z pthread_mutex_lock");
        return;
    }

if (ptr->babble_total < BABBLE_LIMIT) {
        strcpy(ptr->babbles[(ptr->babble_first + ptr->babble_total) % BABBLE_LIMIT], komunikat);
        ptr->babble_total++;
    } else {
        strcpy(ptr->babbles[ptr->babble_first], komunikat);
        ptr->babble_first = (ptr->babble_first + 1) % BABBLE_LIMIT;
    }

    if (pthread_mutex_unlock(&ptr->babble_mutex) != 0) {
        perror("Problem z pthread_mutex_unlock");
    }


}

void *sluchaj_komunikatow(void *arg) {
    struct babblespace *ptr = (struct babblespace *)arg;

    while (1) {
        if (pthread_mutex_lock(&ptr->babble_mutex) != 0) {
            perror("Problem z pthread_mutex_lock");
            return NULL;
        }

        if (pthread_cond_wait(&ptr->babble_cond, &ptr->babble_mutex) != 0) {
            perror("Problem z pthread_cond_wait");
            return NULL;
        }

        wyswietl_komunikaty(ptr);

        if (pthread_mutex_unlock(&ptr->babble_mutex) != 0) {
            perror("Problem z pthread_mutex_unlock");
            return NULL;
        }
    }

    return NULL;
}

int main() {
    int fd = shm_open(BABBLE_NAME, O_RDWR, 0);
    if (fd == -1) {
        perror("Problem z shm_open");
        exit(EXIT_FAILURE);
    }

    
    struct babblespace *ptr = (struct babblespace *) mmap(NULL, sizeof(struct babblespace),
                                                          PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("Problem z mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    char komunikat[BABBLE_LENGTH];
    char nick[20];
	 strcat(nick,"[");
	 strcat(nick,NICKNAME);
	 strcat(nick,"]");

    pid_t pid = fork();

    if (pid == -1) {
        perror("Problem z fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        sluchaj_komunikatow(ptr);
        exit(0);
    }

   // Proces rodzic: odpowiada za dodawanie komunikatów
    while (1) {
        printf("Wpisz komunikat (max %d znaków): ", BABBLE_LENGTH - 1);
        fgets(komunikat, sizeof(komunikat), stdin);
        komunikat[strcspn(komunikat, "\n")] = '\0'; 

        if (strlen(komunikat) == 0) {
           
            break;
        }

        
        char komunikat_z_nickiem[BABBLE_LENGTH];
        snprintf(komunikat_z_nickiem, BABBLE_LENGTH, "%s %s", nick, komunikat);

        dodaj_komunikat(ptr, komunikat_z_nickiem);
    }


    munmap(ptr, sizeof(struct babblespace));
    close(fd);

    wait(NULL);

    return 0;
}
