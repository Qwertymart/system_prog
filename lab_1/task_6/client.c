#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MSG_KEY 1234
#define MAX_TEXT 1024

struct msg_buffer {
    long msg_type;
    char text[MAX_TEXT];
};

int main() {
    int msg_id = msgget(MSG_KEY, 0666);
    if (msg_id == -1) {
        perror("msgget failed");
        exit(1);
    }

    struct msg_buffer message;
    struct msg_buffer response;
    message.msg_type = 1;

    printf("Enter absolute file path (separate them by spaces, type 'exit' to quit):\n");

    while (1) {
        printf("> ");
        fgets(message.text, sizeof(message.text), stdin);
        message.text[strcspn(message.text, "\n")] = '\0';

        if (strcmp(message.text, "exit") == 0) {
            break;
        }

        msgsnd(msg_id, &message, sizeof(message.text), 0);

        if (msgrcv(msg_id, &response, sizeof(response.text), 2, 0) == -1) {
            perror("msgrcv failed");
            exit(1);
        }

        printf("\nServer response:\n%s", response.text);
    }

    return 0;
}
