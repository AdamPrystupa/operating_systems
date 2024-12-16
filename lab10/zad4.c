#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <ctype.h>

#define MAX_WORD_LEN 256
#define MAX_HASH_LEN 33
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

int load_hashes_and_emails(const char *filename, char ***hashes, char ***emails) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Nie można otworzyć pliku");
        return -1;
    }

    char **hashes_temp = NULL;
    char **emails_temp = NULL;
    int count = 0;
    char line[MAX_WORD_LEN * 4];

    while (fgets(line, sizeof(line), file)) {
        hashes_temp = realloc(hashes_temp, (count + 1) * sizeof(char *));
        emails_temp = realloc(emails_temp, (count + 1) * sizeof(char *));
        if (!hashes_temp || !emails_temp) {
            perror("Błąd alokacji pamięci");
            fclose(file);
            free(hashes_temp);
            free(emails_temp);
            return -1;
        }

        hashes_temp[count] = malloc(MAX_HASH_LEN);
        emails_temp[count] = malloc(MAX_WORD_LEN);
        if (!hashes_temp[count] || !emails_temp[count]) {
            perror("Błąd alokacji pamięci");
            fclose(file);
            for (int i = 0; i <= count; i++) {
                free(hashes_temp[i]);
                free(emails_temp[i]);
            }
            free(hashes_temp);
            free(emails_temp);
            return -1;
        }

        char *token = strtok(line, COLUMN_SEPARATOR);
        int col = 0;
        while (token != NULL) {
            if (col == 1) {
                strncpy(hashes_temp[count], token, MAX_HASH_LEN - 1);
                hashes_temp[count][strcspn(hashes_temp[count], "\n")] = '\0';
            } else if (col == 2) {
                strncpy(emails_temp[count], token, MAX_WORD_LEN - 1);
                emails_temp[count][strcspn(emails_temp[count], "\n")] = '\0';
            }
            token = strtok(NULL, COLUMN_SEPARATOR);
            col++;
        }
        count++;
    }
    fclose(file);

    *hashes = hashes_temp;
    *emails = emails_temp;
    return count;
}

int load_words_from_file(const char *filename, char ***words) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Nie można otworzyć pliku");
        return -1;
    }

    char **words_temp = NULL;
    int count = 0;
    char line[MAX_WORD_LEN];

    while (fgets(line, MAX_WORD_LEN, file)) {
        words_temp = realloc(words_temp, (count + 1) * sizeof(char *));
        if (!words_temp) {
            perror("Błąd alokacji pamięci");
            fclose(file);
            for (int i = 0; i < count; i++) {
                free(words_temp[i]);
            }
            free(words_temp);
            return -1;
        }

        words_temp[count] = malloc(MAX_WORD_LEN);
        if (!words_temp[count]) {
            perror("Błąd alokacji pamięci");
            fclose(file);
            for (int i = 0; i < count; i++) {
                free(words_temp[i]);
            }
            free(words_temp);
            return -1;
        }

        strncpy(words_temp[count], line, MAX_WORD_LEN - 1);
        words_temp[count][strcspn(words_temp[count], "\n")] = '\0';
        count++;
    }
    fclose(file);

    *words = words_temp;
    return count;
}

void to_uppercase(const char *input, char *output) {
    for (int i = 0; input[i] != '\0'; i++) {
        output[i] = toupper(input[i]);
    }
    output[strlen(input)] = '\0';
}

void capitalize(const char *input, char *output) {
    if (input[0] != '\0') {
        output[0] = toupper(input[0]);
        for (int i = 1; input[i] != '\0'; i++) {
            output[i] = tolower(input[i]);
        }
        output[strlen(input)] = '\0';
    }
}

void check_password_variants(const char *word, char **hashed_passwords, char **emails, int num_passwords) {
    char hash[MAX_HASH_LEN];
    char modified_word[MAX_WORD_LEN];
    char variant_word[MAX_WORD_LEN];

    char lowercase_word[MAX_WORD_LEN];
    char uppercase_word[MAX_WORD_LEN];
    char capitalized_word[MAX_WORD_LEN];

    to_uppercase(word, uppercase_word);
    capitalize(word, capitalized_word);

    const char *base_variants[] = {word, lowercase_word, uppercase_word, capitalized_word};
    for (int k = 0; k < 4; k++) {
        bytes2md5(base_variants[k], strlen(base_variants[k]), hash);
        for (int j = 0; j < num_passwords; j++) {
            if (strcmp(hash, hashed_passwords[j]) == 0) {
                printf("Password for %s is %s\n", emails[j], base_variants[k]);
            }
        }
    }

    for (int prefix = 0; prefix < MAX_PREFIX_POSTFIX; prefix++) {
        for (int postfix = 0; postfix < MAX_PREFIX_POSTFIX; postfix++) {
            for (int k = 0; k < 4; k++) {
                if (prefix == 0 && postfix == 0) continue;

                if (prefix > 0 && postfix > 0) {
                    snprintf(modified_word, MAX_WORD_LEN, "%d%s%d", prefix, base_variants[k], postfix);
                } else if (prefix > 0) {
                    snprintf(modified_word, MAX_WORD_LEN, "%d%s", prefix, base_variants[k]);
                } else {
                    snprintf(modified_word, MAX_WORD_LEN, "%s%d", base_variants[k], postfix);
                }

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
    char **hashed_passwords;
    char **emails;
    char **dictionary;

    int num_passwords = load_hashes_and_emails("test-data3.txt", &hashed_passwords, &emails);
    if (num_passwords == -1) return 1;

    int num_words = load_words_from_file("test-dict-mini.txt", &dictionary);
    if (num_words == -1) {
        for (int i = 0; i < num_passwords; i++) {
            free(hashed_passwords[i]);
            free(emails[i]);
        }
        free(hashed_passwords);
        free(emails);
        return 1;
    }

    for (int i = 0; i < num_words; i++) {
        check_password_variants(dictionary[i], hashed_passwords, emails, num_passwords);
    }

    for (int i = 0; i < num_passwords; i++) {
        free(hashed_passwords[i]);
        free(emails[i]);
    }
    free(hashed_passwords);
    free(emails);

    for (int i = 0; i < num_words; i++) {
        free(dictionary[i]);
    }
    free(dictionary);

    return 0;
}
