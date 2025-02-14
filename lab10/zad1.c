#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>

#define MAX_WORD_LEN 256
#define MAX_HASH_LEN 33
#define MAX_LINES 1000
#define COLUMN_SEPARATOR "\t"

void bytes2md5(const char *data, int len, char *md5buf) {
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    const EVP_MD *md = EVP_md5();
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len, i;
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, data, len);
    EVP_DigestFinal_ex(mdctx, md_value, &md_len);
    EVP_MD_CTX_free(mdctx);
    for (i = 0; i < md_len; i++) {
        snprintf(&(md5buf[i * 2]), 16 * 2, "%02x", md_value[i]);
    }
}


int load_hashes_and_emails(const char *filename, char hashes[MAX_LINES][MAX_HASH_LEN], char emails[MAX_LINES][MAX_WORD_LEN]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Nie mozna otworzyc pliku");
        return -1;
    }

    int count = 0;
    char line[MAX_WORD_LEN * 4]; 
    while (fgets(line, sizeof(line), file) && count < MAX_LINES) {
        char *token = strtok(line, COLUMN_SEPARATOR); 
        int col = 0;
        while (token != NULL) {
            if (col == 1) {
                strncpy(hashes[count], token, MAX_HASH_LEN - 1);
                hashes[count][strcspn(hashes[count], "\n")] = '\0'; 
            } else if (col == 2) { 
                strncpy(emails[count], token, MAX_WORD_LEN - 1);
                emails[count][strcspn(emails[count], "\n")] = '\0';
            }
            token = strtok(NULL, COLUMN_SEPARATOR);
            col++;
        }
        count++;
    }
    fclose(file);
    return count;
}


int load_words_from_file(const char *filename, char words[MAX_LINES][MAX_WORD_LEN]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Nie mozna otworzyc pliku");
        return -1;
    }

    int count = 0;
    while (fgets(words[count], MAX_WORD_LEN, file) && count < MAX_LINES) {
        words[count][strcspn(words[count], "\n")] = '\0'; 
        count++;
    }
    fclose(file);
    return count;
}

int main() {
    char hashed_passwords[MAX_LINES][MAX_HASH_LEN];
    char emails[MAX_LINES][MAX_WORD_LEN];
    char dictionary[MAX_LINES][MAX_WORD_LEN];
    char hash[MAX_HASH_LEN];

  
    int num_passwords = load_hashes_and_emails("test-data1.txt", hashed_passwords, emails);
    if (num_passwords == -1) return 1;

    int num_words = load_words_from_file("test-dict-mini.txt", dictionary);
    if (num_words == -1) return 1;

    for (int i = 0; i < num_words; i++) {
        bytes2md5(dictionary[i], strlen(dictionary[i]), hash);
        for (int j = 0; j < num_passwords; j++) {
            if (strcmp(hash, hashed_passwords[j]) == 0) {
                printf("Password for %s is %s\n", emails[j], dictionary[i]);
            }
        }
    }

    return 0;
}
