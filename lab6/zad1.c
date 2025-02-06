#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 256
#define PACKAGE_SIZE 16

int main(int argc, char *argv[]) {

    int pipefd[2]; 
    pipe(pipefd);
    pid_t pid = fork();

    if (pid == 0) { 
        close(pipefd[1]);

        char buffer[PACKAGE_SIZE + 1];
        ssize_t bytesRead;

        
        while ((bytesRead = read(pipefd[0], buffer, PACKAGE_SIZE)) > 0) {
            buffer[bytesRead] = '\0'; 
            printf("@@@@%s####\n", buffer);
        }

        close(pipefd[0]); 
        exit(0);
    } else { 
        close(pipefd[0]);

        
        FILE *file = fopen(argv[1], "r");

        char buffer[BUFFER_SIZE]; 
        while (fgets(buffer, sizeof(buffer), file) != NULL) {
            size_t len = strlen(buffer);
            size_t offset = 0;
            size_t toSend = 0;
            
            while (offset < len) {
                if(len-offset<PACKAGE_SIZE)
                {
                    toSend=len-offset;
                }
                else
                toSend=PACKAGE_SIZE;
                write(pipefd[1], buffer + offset, toSend); 
                offset += toSend; 
            }
        }

        fclose(file); 
        close(pipefd[1]);

        wait(NULL);
    }

    return 0;
}
