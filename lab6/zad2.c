#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Użycie: %s <ścieżka do pliku graficznego>\n", argv[0]);
        return 1;
    }

    int pipefd[2]; // Tablica do utworzenia potoku
    if (pipe(pipefd) == -1) { // Utworzenie potoku
        perror("pipe");
        return 1;
    }

    pid_t pid = fork(); // Utworzenie procesu potomnego
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) { // Proces potomny
        close(pipefd[1]); // Zamknij związek do zapisu

        // Przekierowanie potoku na standardowe wejście
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]); // Zamknij nieużywany deskryptor potoku

        // Uruchomienie programu do wyświetlania obrazów
        execlp("display", "display", "-update", "1", NULL); // Przykład użycia ImageMagick
        // Można też użyć innych programów: qiv, feh, eog, eom
        perror("execlp"); // Wypisanie błędu, jeśli exec się nie powiedzie
        exit(1);
    } else { // Proces nadrzędny
        close(pipefd[0]); // Zamknij związek do odczytu

        // Otwórz plik graficzny
        FILE *file = fopen(argv[1], "rb"); // Otwórz w trybie binarnym
        if (file == NULL) {
            perror("fopen");
            close(pipefd[1]);
            return 1;
        }

        // Przesyłanie danych pliku graficznego przez potok
        char buffer[BUFFER_SIZE];
        size_t bytesRead;

        while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            write(pipefd[1], buffer, bytesRead); // Wysyłanie danych przez potok
        }

        fclose(file); // Zamknij plik
        close(pipefd[1]); // Zamknij związek do zapisu

        wait(NULL); // Czekaj na zakończenie procesu potomnego
    }

    return 0;
}
