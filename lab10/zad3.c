#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <ctype.h>

#define MAX_WORD_LEN 256
#define MAX_HASH_LEN 33
#define MAX_LINES 1000
#define COLUMN_SEPARATOR "\t"
#define MAX_PREFIX_POSTFIX 100

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
        perror("Nie można otworzyć pliku");
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
        perror("Nie można otworzyć pliku");
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

// Funkcja zmienia wszystkie litery na małe
void to_lowercase(const char *input, char *output) {
    for (int i = 0; input[i] != '\0'; i++) {
        output[i] = tolower(input[i]);
    }
    output[strlen(input)] = '\0';
}

// Funkcja zmienia wszystkie litery na wielkie
void to_uppercase(const char *input, char *output) {
    for (int i = 0; input[i] != '\0'; i++) {
        output[i] = toupper(input[i]);
    }
    output[strlen(input)] = '\0';
}

// Funkcja zamienia pierwszą literę na wielką, reszta pozostaje mała
void capitalize(const char *input, char *output) {
    if (input[0] != '\0') {
        output[0] = toupper(input[0]);
        for (int i = 1; input[i] != '\0'; i++) {
            output[i] = tolower(input[i]);
        }
        output[strlen(input)] = '\0';
    }
}

void check_password_variants(const char *word, char hashed_passwords[MAX_LINES][MAX_HASH_LEN], char emails[MAX_LINES][MAX_WORD_LEN], int num_passwords) {
    char hash[MAX_HASH_LEN];
    char modified_word[MAX_WORD_LEN];
    char variant_word[MAX_WORD_LEN];

    // Warianty bazowe dla słowa
    char lowercase_word[MAX_WORD_LEN];
    char uppercase_word[MAX_WORD_LEN];
    char capitalized_word[MAX_WORD_LEN];

    to_lowercase(word, lowercase_word);
    to_uppercase(word, uppercase_word);
    capitalize(word, capitalized_word);

    // Sprawdzenie podstawowego słowa w trzech konfiguracjach wielkich liter
    const char *base_variants[] = {word, lowercase_word, uppercase_word, capitalized_word};
    for (int k = 0; k < 4; k++) {
        bytes2md5(base_variants[k], strlen(base_variants[k]), hash);
        for (int j = 0; j < num_passwords; j++) {
            if (strcmp(hash, hashed_passwords[j]) == 0) {
                printf("Password for %s is %s\n", emails[j], base_variants[k]);
            }
        }
    }

    // Prefiksy i sufiksy (0–9999 dla obu)
    for (int prefix = 0; prefix < MAX_PREFIX_POSTFIX; prefix++) {
        for (int postfix = 0; postfix < MAX_PREFIX_POSTFIX; postfix++) {
            for (int k = 0; k < 4; k++) { // Przetwarzanie wszystkich wariantów liter
                if (prefix == 0 && postfix == 0) continue;

                // Budowanie słowa z prefiksem i sufiksem
                if (prefix > 0 && postfix > 0) {
                    snprintf(modified_word, MAX_WORD_LEN, "%d%s%d", prefix, base_variants[k], postfix);
                } else if (prefix > 0) {
                    snprintf(modified_word, MAX_WORD_LEN, "%d%s", prefix, base_variants[k]);
                } else {
                    snprintf(modified_word, MAX_WORD_LEN, "%s%d", base_variants[k], postfix);
                }

                // Sprawdzenie wariantu
                bytes2md5(modified_word, strlen(modified_word), hash);
                for (int j = 0; j < num_passwords; j++) {
                    if (strcmp(hash, hashed_passwords[j]) == 0) {
                        printf("Password for %s is %s\n", emails[j], modified_word);
                    }
                }
            }
        }
    }
}



int main() {
    char hashed_passwords[MAX_LINES][MAX_HASH_LEN];
    char emails[MAX_LINES][MAX_WORD_LEN];
    char dictionary[MAX_LINES][MAX_WORD_LEN];

    int num_passwords = load_hashes_and_emails("hdz3.txt", hashed_passwords, emails);
    if (num_passwords == -1) return 1;

    int num_words = load_words_from_file("msph123.txt", dictionary);
    if (num_words == -1) return 1;

    for (int i = 0; i < num_words; i++) {
        check_password_variants(dictionary[i], hashed_passwords, emails, num_passwords);
    }

    return 0;
}