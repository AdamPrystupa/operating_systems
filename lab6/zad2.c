#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

int main(int argc, char *argv[]) {

    int pipefd[2]; 
    pipe(pipefd);
    pid_t pid = fork(); 


    if (pid == 0) { 
        close(pipefd[1]); 


        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);


        execlp("display", "display", "-update", "1", NULL); 
        exit(1);
    } else { 
        close(pipefd[0]); 

      
        FILE *file = fopen(argv[1], "rb");

        char buffer[BUFFER_SIZE];
        size_t bytesRead;

        while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            write(pipefd[1], buffer, bytesRead); 
        }

        fclose(file); 
        close(pipefd[1]); 

        wait(NULL); 
    }

    return 0;
}