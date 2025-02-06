#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256

void process_line(const char *line) {
    int minute;
    char buffer[MAX_LINE_LENGTH];
    
    strncpy(buffer, line, MAX_LINE_LENGTH);
    buffer[MAX_LINE_LENGTH - 1] = '\0';

    char *token = strtok(buffer, " ");
    if (token == NULL) return;
    minute = atoi(token);

    printf("Moment %d:\n", minute);

    while ((token = strtok(NULL, " ")) != NULL) {
        int cart_number;
        char stone_type[32];
        int weight, quantity;

        cart_number = atoi(token);

        token = strtok(NULL, " ");
        if (token == NULL) break;
        strncpy(stone_type, token, sizeof(stone_type));
        stone_type[sizeof(stone_type) - 1] = '\0';

        token = strtok(NULL, " ");
        if (token == NULL) break;
        weight = atoi(token);

        token = strtok(NULL, " ");
        if (token == NULL) break;
        quantity = atoi(token);

        int unload_time = weight * quantity;

        printf("  wheelbarrow %d: %d %s stones (weight %d) -> %d minutes\n",
               cart_number, quantity, stone_type, weight, unload_time);
    }
}

int main() {
    FILE *file = fopen("dane.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    char line[MAX_LINE_LENGTH];

    printf("Processing taczki data:\n\n");
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';

        process_line(line);
        printf("\n");
    }

    fclose(file);
    return 0;
}
