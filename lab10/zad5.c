#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <openssl/evp.h>
#include <ctype.h>

#define MAX_WORD_LEN 256
#define MAX_HASH_LEN 33
#define COLUMN_SEPARATOR "\t"
#define MAX_PREFIX_POSTFIX 100
#define NUM_VARIANTS 3


pthread_mutex_t lock;
pthread_cond_t cond;
int producer_done = 0;


typedef struct {
    char **hashed_passwords;
    char **emails;
    int num_passwords;
    char **dictionary;
    int num_words;
    int variant_index;
} producer_data_t;

typedef struct {
    char password[MAX_WORD_LEN];
    char email[MAX_WORD_LEN];
    int thread_id;
} found_password_t;

found_password_t *found_passwords;
int found_count = 0;
int max_passwords;

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

void *producer(void *arg) {
    producer_data_t *data = (producer_data_t *)arg;
    int thread_id = data->variant_index;

    char hash[MAX_HASH_LEN];
    char modified_word[MAX_WORD_LEN];
    char base_word[MAX_WORD_LEN];

    for (int i = 0; i < data->num_words; i++) {
        const char *original_word = data->dictionary[i];
        switch (data->variant_index) {
            case 0: 
                strncpy(base_word, original_word, MAX_WORD_LEN);
                break;
            case 1: 
                to_uppercase(original_word, base_word);
                break;
            case 2: 
                capitalize(original_word, base_word);
                break;
        }

        for (int prefix = 0; prefix < MAX_PREFIX_POSTFIX; prefix++) {
            for (int postfix = 0; postfix < MAX_PREFIX_POSTFIX; postfix++) {
                if (found_count >= max_passwords) break;

                if (prefix == 0 && postfix == 0) {
                    snprintf(modified_word, MAX_WORD_LEN, "%s", base_word);
                } else if (prefix > 0 && postfix > 0) {
                    snprintf(modified_word, MAX_WORD_LEN, "%d%s%d", prefix, base_word, postfix);
                } else if (prefix > 0) {
                    snprintf(modified_word, MAX_WORD_LEN, "%d%s", prefix, base_word);
                } else {
                    snprintf(modified_word, MAX_WORD_LEN, "%s%d", base_word, postfix);
                }

                bytes2md5(modified_word, strlen(modified_word), hash);

                pthread_mutex_lock(&lock);
                for (int j = 0; j < data->num_passwords; j++) {
                    if (strcmp(hash, data->hashed_passwords[j]) == 0) {
                        if (found_count < max_passwords) {
                            strncpy(found_passwords[found_count].password, modified_word, MAX_WORD_LEN);
                            strncpy(found_passwords[found_count].email, data->emails[j], MAX_WORD_LEN);
                            found_passwords[found_count].thread_id = thread_id;
                            found_count++;
                        }
                    }
                }
                pthread_mutex_unlock(&lock);
            }
        }
    }

    pthread_mutex_lock(&lock);
    producer_done++;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);

    return NULL;
}

void *consumer(void *arg) {
    pthread_mutex_lock(&lock);
    while (producer_done < NUM_VARIANTS) {
        pthread_cond_wait(&cond, &lock);
    }

    for (int i = 0; i < found_count; i++) {
        printf("Thread %d found password for %s: %s\n", found_passwords[i].thread_id, found_passwords[i].email, found_passwords[i].password);
    }

    if (found_count == 0) {
        printf("No passwords found.\n");
    }
    pthread_mutex_unlock(&lock);
    return NULL;
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

    max_passwords = num_passwords;
    found_passwords = calloc(max_passwords, sizeof(found_password_t));

    pthread_t producers[NUM_VARIANTS];
    pthread_t consumer_thread;

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);

    producer_data_t producer_data[NUM_VARIANTS];
    for (int i = 0; i < NUM_VARIANTS; i++) {
        producer_data[i].hashed_passwords = hashed_passwords;
        producer_data[i].emails = emails;
        producer_data[i].num_passwords = num_passwords;
        producer_data[i].dictionary = dictionary;
        producer_data[i].num_words = num_words;
        producer_data[i].variant_index = i;
        pthread_create(&producers[i], NULL, producer, &producer_data[i]);
    }

    pthread_create(&consumer_thread, NULL, consumer, NULL);

    for (int i = 0; i < NUM_VARIANTS; i++) {
        pthread_join(producers[i], NULL);
    }

    pthread_join(consumer_thread, NULL);

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

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
    free(found_passwords);

    return 0;
}
