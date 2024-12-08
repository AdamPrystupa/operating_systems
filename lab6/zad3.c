#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#define FIFO_PATH "/tmp/image_fifo"  // Ścieżka do potoku FIFO
#define BUFFER_SIZE 256
#define DELAY_SECONDS 5

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Użycie: %s <plik1> <plik2> ... <plikN>\n", argv[0]);
        return 1;
    }

    // Utworzenie potoku FIFO, jeśli jeszcze nie istnieje
    if (mkfifo(FIFO_PATH, 0666) == -1) {
        if (errno != EEXIST) { // Ignoruj błąd, jeśli FIFO już istnieje
            perror("mkfifo");
            return 1;
        }
    }

    // Otwórz potok do zapisu
    int fifo_fd = open(FIFO_PATH, O_WRONLY);
    if (fifo_fd == -1) {
        perror("open");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        // Otwórz plik graficzny w trybie binarnym
        FILE *file = fopen(argv[i], "rb");
        if (file == NULL) {
            perror("fopen");
            continue;  // Pomijamy ten plik i przechodzimy do następnego
        }

        // Przesyłanie zawartości pliku do FIFO
        char buffer[BUFFER_SIZE];
        size_t bytesRead;

        while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            if (write(fifo_fd, buffer, bytesRead) == -1) {
                perror("write");
                fclose(file);
                close(fifo_fd);
                return 1;
            }
        }

        fclose(file); // Zamknij plik po przesłaniu danych

        // Odczekaj 5 sekund przed przesyłaniem kolejnego pliku
        sleep(DELAY_SECONDS);
    }

    close(fifo_fd); // Zamknij potok FIFO
    return 0;
}
