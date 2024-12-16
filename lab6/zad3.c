#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 256
#define PACKAGE_SIZE 50

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Użycie: %s <ścieżka do pliku1> <ścieżka do pliku2> ...\n", argv[0]);
        return 1;
    }

    const char *fifo_path = "/tmp/my-fifo";

    if (mkfifo(fifo_path, 0666) == -1) {
        perror("mkfifo");
        return 1;
    }

    int fifo_fd = open(fifo_path, O_WRONLY);
    if (fifo_fd == -1) {
        perror("open");
        return 1;
    }

    FILE **files = malloc((argc - 1) * sizeof(FILE *));
    size_t *offsets = malloc((argc - 1) * sizeof(size_t));
    int files_remaining = 0;

    for (int i = 1; i < argc; i++) {
        FILE *file = fopen(argv[i], "r");
        if (file == NULL) {
            perror("fopen");
            files[i - 1] = NULL;
            offsets[i - 1] = 0;
            continue;
        }
        files[i - 1] = file;
        offsets[i - 1] = 0;
        files_remaining++;
        char header[BUFFER_SIZE];
        snprintf(header, sizeof(header), "### START PLIKU %d: %s ###\n", i, argv[i]);
        write(fifo_fd, header, strlen(header));
    }

    char buffer[PACKAGE_SIZE + 1];

    while (files_remaining > 0) {
        for (int i = 0; i < argc - 1; i++) {
            if (files[i] == NULL) {
                continue;
            }

            size_t bytes_read = fread(buffer, 1, PACKAGE_SIZE, files[i]);
            if (bytes_read > 0) {
                char packet[BUFFER_SIZE + 1];
                snprintf(packet, sizeof(packet), "PLIK %d: %.*s\n", i + 1, (int)bytes_read, buffer);
                write(fifo_fd, packet, strlen(packet));
            } else {
                fclose(files[i]);
                files[i] = NULL;
                files_remaining--;
            }
            sleep(1);
        }
    }

    free(files);
    free(offsets);


    const char *end_message = "### KONIEC PRZESYŁANIA ###\n";
    write(fifo_fd, end_message, strlen(end_message));

    close(fifo_fd);
    unlink(fifo_path);

    return 0;
}
