#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#define MSG_KEY 1234
#define MAX_TEXT 1024

struct msg_buffer {
    long msg_type;
    char text[MAX_TEXT];
};

void process_message(int msg_id) {
    struct msg_buffer message;
    struct msg_buffer response;
    response.msg_type = 2;

    while (1) {
        if (msgrcv(msg_id, &message, sizeof(message.text), 1, 0) == -1) {
            perror("msgrcv failed");
            exit(1);
        }

        if (strcmp(message.text, "exit") == 0) {
            printf("Server exiting...\n");
            msgctl(msg_id, IPC_RMID, NULL);
            exit(0);
        }

        printf("Received file paths: %s\n", message.text);

        char *token = message.text;
        char directory[MAX_TEXT];
        char *last_slash = strrchr(token, '/');
        if (last_slash != NULL) {
            size_t dir_length = last_slash - token;
            if (dir_length < MAX_TEXT) {
                strncpy(directory, token, dir_length);
                directory[dir_length] = '\0';
            } else {
                strcpy(response.text, "Path too long\n");
                msgsnd(msg_id, &response, sizeof(response.text), 0);
                continue;
            }
        } else {
            strcpy(response.text, "Invalid file path\n");
            msgsnd(msg_id, &response, sizeof(response.text), 0);
            continue;
        }

        struct stat path_stat;
        if (stat(directory, &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
            snprintf(response.text, sizeof(response.text), "\n%s:\n", directory);

            DIR *dir = opendir(directory);
            if (dir) {
                struct dirent *entry;
                while ((entry = readdir(dir)) != NULL) {
                    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                        continue;
                    }
                    snprintf(response.text + strlen(response.text), sizeof(response.text) - strlen(response.text),
                             "  - %s\n", entry->d_name);
                }
                closedir(dir);
            }
        } else {
            snprintf(response.text, sizeof(response.text), "Directory does not exist: %s\n", directory);
        }

        msgsnd(msg_id, &response, sizeof(response.text), 0);
    }
}

int main() {
    int msg_id = msgget(MSG_KEY, 0666);

    if (msg_id != -1) {
        if (msgctl(msg_id, IPC_RMID, NULL) == -1) {
            perror("msgctl failed");
            exit(1);
        }
    }

    msg_id = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msg_id == -1) {
        perror("msgget failed");
        exit(1);
    }

    printf("Server is running...\n");
    process_message(msg_id);
    return 0;
}
