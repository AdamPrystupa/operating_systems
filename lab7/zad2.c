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

struct babblespace {
    pthread_mutex_t babble_mutex;
    pthread_cond_t babble_cond;
    int babble_first, babble_total;
    char babbles[BABBLE_LIMIT][BABBLE_LENGTH];
};



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


    if (pthread_mutex_lock(&ptr->babble_mutex) != 0) {
        perror("Problem z pthread_mutex_lock");
        munmap(ptr, sizeof(struct babblespace));
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("Wyświetlanie komunikatów:\n");
    for (int i = 0; i < ptr->babble_total; i++) {
        int index = (ptr->babble_first + i) % BABBLE_LIMIT;
        printf("%s\n", ptr->babbles[index]);
    }

    if (pthread_mutex_unlock(&ptr->babble_mutex) != 0) {
        perror("Problem z pthread_mutex_unlock");
    }


    munmap(ptr, sizeof(struct babblespace));
    close(fd);

    return 0;
}
