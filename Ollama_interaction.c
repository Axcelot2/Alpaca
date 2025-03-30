#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <windows.h>
#include <direct.h>

#define RESPONSE_BUFFER_SIZE 8192
#define PROMPT_BUFFER_SIZE 2048

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total_size = size * nmemb;
    char *chunk = (char *)ptr;
    chunk[total_size] = '\0';

    char *start = strstr(chunk, "\"response\":\"");
    if (start) {
        start += strlen("\"response\":\"");
        char *end = strchr(start, '"');
        if (end) {
            *end = '\0';
            for (char *p = start; *p; ++p) {
                if (p[0] == '\\' && p[1] == 'n') {
                    p[0] = '\n';
                    memmove(p + 1, p + 2, strlen(p + 2) + 1);
                }
            }
            printf("%s", start);
            *end = '"';
        }
    }

    return total_size;
}

void send_prompt_to_ollama(const char *prompt) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        printf("Failed to initialize libcurl.\n");
        return;
    }

    char json_payload[PROMPT_BUFFER_SIZE + 128];
    snprintf(json_payload, sizeof(json_payload),
             "{ \"model\": \"mistral\", \"prompt\": \"%s\", \"stream\": false }",
             prompt);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/generate");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(res));
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

void list_files_and_dirs(char items[][260], int *count, int *is_dir_flags) {
    WIN32_FIND_DATAA fileData;
    HANDLE hFind = FindFirstFileA("*", &fileData);
    *count = 0;

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(fileData.cFileName, ".") != 0 && strcmp(fileData.cFileName, "..") != 0) {
                strcpy(items[*count], fileData.cFileName);
                is_dir_flags[*count] = (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
                (*count)++;
            }
        } while (FindNextFileA(hFind, &fileData));
        FindClose(hFind);
    }
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);

    int choice;
    char prompt[PROMPT_BUFFER_SIZE];

    while (1) {
        char cwd[260];
        _getcwd(cwd, sizeof(cwd));
        printf("\nCurrent Directory: %s\n", cwd);

        printf("\nMenu:\n");
        printf("1. Upload example.txt into LLM\n");
        printf("2. Chat with model\n");
        printf("3. Browse and select file/folder\n");
        printf("0. Quit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();  // flush newline

        if (choice == 0) {
            printf("Exiting...\n");
            break;
        } else if (choice == 1) {
            FILE *fp = fopen("example.txt", "r");
            if (!fp) {
                printf("Failed to open example.txt\n");
                continue;
            }
            size_t len = fread(prompt, 1, PROMPT_BUFFER_SIZE - 1, fp);
            prompt[len] = '\0';
            fclose(fp);

            send_prompt_to_ollama(prompt);
        } else if (choice == 2) {
            printf("Enter your question: ");
            fgets(prompt, sizeof(prompt), stdin);
            prompt[strcspn(prompt, "\n")] = '\0';
            send_prompt_to_ollama(prompt);
        } else if (choice == 3) {
            char items[200][260];
            int is_dir[200];
            int item_count;

            while (1) {
                list_files_and_dirs(items, &item_count, is_dir);
                printf("\nItems in current directory:\n");
                printf("0. Go back\n");
                for (int i = 0; i < item_count; i++) {
                    printf("%d. %s%s\n", i + 1, items[i], is_dir[i] ? "/" : "");
                }

                printf("Select a file or folder: ");
                int selection;
                scanf("%d", &selection);
                getchar();

                if (selection == 0) break;
                if (selection < 1 || selection > item_count) {
                    printf("Invalid selection.\n");
                    continue;
                }

                if (is_dir[selection - 1]) {
                    _chdir(items[selection - 1]);
                } else {
                    FILE *fp = fopen(items[selection - 1], "r");
                    if (!fp) {
                        printf("Failed to open file.\n");
                        continue;
                    }
                    size_t len = fread(prompt, 1, PROMPT_BUFFER_SIZE - 1, fp);
                    prompt[len] = '\0';
                    fclose(fp);

                    send_prompt_to_ollama(prompt);
                    break;
                }
            }
        } else {
            printf("Invalid choice.\n");
        }
    }

    curl_global_cleanup();
    return 0;
}
