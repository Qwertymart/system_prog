#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include "string.h"
#include <limits.h>
#include <ctype.h>
#include <stdlib.h>
#define FILENAME "users.txt"


typedef struct User
{
    char login[7];
    int pin;
    int sanctions;
}User;

typedef struct {
    User user;
    int request_count;
} Session;

void menu_authorization()
{
    printf("Press:\n"
           "1 - sing up\n"
           "2 - log in\n"
           "3 - exit\n");
}

void clear_input_buffer()
{
    char c;
    while ((c = getchar()) != '\n' && c != EOF);
}

char is_valid_login(const char* login)
{
    for (int i = 0; login[i] != '\0'; i++)
    {
        if (!isalnum(login[i]))
        {
            return 0;
        }
    }
    return 1;
}

int is_login_taken(const char* login)
{
    FILE* file = fopen(FILENAME, "r");
    if (file == NULL)
    {
        return 0;
    }

    User user;
    while (fscanf(file, "%6s %d %d", user.login, &user.pin, &user.sanctions) == 3)
    {
        if (strcmp(user.login, login) == 0)
        {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}


int sign_up(const char* login, int pin)
{
    if (is_login_taken(login))
    {
        return 2;
    }

    FILE* file = fopen(FILENAME, "a");
    if (file == NULL)
    {
        return 1;
    }

    User new_user;
    strcpy(new_user.login, login);
    new_user.pin = pin;
    new_user.sanctions = -1;

    fprintf(file, "%s %d %d\n", new_user.login, new_user.pin, new_user.sanctions);
    fclose(file);

    return 0;
}

int log_in(const char* login, int pin, User* current)
{
    FILE* file = fopen(FILENAME, "r");
    if (file == NULL)
    {
        return 1;
    }

    User user;
    while (fscanf(file, "%6s %d %d", user.login, &user.pin, &user.sanctions) == 3)
    {
        if (strcmp(user.login, login) == 0 && user.pin == pin)
        {
            strcpy(current->login, user.login);
            current->pin = user.pin;
            current->sanctions = user.sanctions;
            fclose(file);
            return 0;
        }
    }

    fclose(file);
    return 1;
}

time_t parse_date(const char* date_str)
{
    if (date_str == NULL)
    {
        return -1;
    }

    struct tm tm = {0};

    if (sscanf(date_str, "%d:%d:%d", &tm.tm_mday, &tm.tm_mon, &tm.tm_year) != 3)
    {
        return -1;
    }

    tm.tm_mon -= 1;
    tm.tm_year -= 1900;

    time_t result = mktime(&tm);

    if (result == -1)
    {
        return -1;
    }

    return result;
}

int string_to_int(const char *str, int *result)
{
    char *endinp;
    if(strlen(str) > 12)
    {
        return 1;
    }
    *result = strtol(str, &endinp, 10);
    if (*result == INT_MAX || *result == INT_MIN)
        return 1;
    if (*endinp != '\0')
        return 1;
    return 0;
}

int update_sanctions(const char* login, int new_sanctions) {
    FILE* file = fopen(FILENAME, "r");
    if (file == NULL)
    {
        return 1;
    }

    FILE* temp_file = fopen("temp_users.txt", "w");
    if (temp_file == NULL) {
        fclose(file);
        return 1;
    }
    User user;
    int updated = 0;

    while (fscanf(file, "%6s %d %d", user.login, &user.pin, &user.sanctions) == 3) {
        if (strcmp(user.login, login) == 0)
        {
            user.sanctions = new_sanctions;
            updated = 1;
        }

        fprintf(temp_file, "%s %d %d\n", user.login, user.pin, user.sanctions);
    }

    fclose(file);
    fclose(temp_file);

    if (updated)
    {
        remove(FILENAME);
        rename("temp_users.txt", FILENAME);
    } else
    {
        remove("temp_users.txt");
    }
    return 0;
}

void show_menu() {
    printf("\nAvailable commands:\n");
    printf("1. Time - Get current time (hh:mm:ss)\n");
    printf("2. Date - Get current date (dd:mm:yyyy)\n");
    printf("3. Howmuch <time> <flag> - Calculate elapsed time since <time> (-s: seconds, -m: minutes, -h: hours, -y: years)\n");
    printf("4. Logout - Return to authorization menu\n");
    printf("5. Sanctions <username> <number> - Set request limit for <username> (confirmation code required)\n");
    printf("Enter your choice: ");
}

void sessions(Session* session)
{

    printf("Welcome, %s!\n", session->user.login);

    char command[51];

    while(1)
    {
        if (!(session->user.sanctions == -1 || session->request_count <= session->user.sanctions))
        {
            break;
        }
        show_menu();

        fgets(command, 51, stdin);
        command[strcspn(command, "\n")] = 0;
        if (strlen(command) > 50)
        {
            printf("Input error\n");
            continue;
        }


        if(session->user.sanctions != -1 &&
           session->request_count >= session->user.sanctions)
        {
            return;
        }

        if (strcmp(command, "Time") == 0)
        {
            time_t now = time(NULL);
            struct tm* t = localtime(&now);
            printf("%02d:%02d:%02d\n", t->tm_hour, t->tm_min, t->tm_sec);
            session->request_count++;
        }

        else if(strcmp(command, "Date") == 0)
        {
            time_t now = time(NULL);
            struct tm* t = localtime(&now);
            printf("%02d:%02d:%04d\n", t->tm_mday, t->tm_mon + 1, t->tm_year + 1900);
            session->request_count++;
        }

        else if(strncmp(command, "Howmuch", 7) == 0)
        {
            char date_str[20];
            char flag[3];
            if(sscanf(command, "Howmuch %19s %2s", date_str, flag) != 2)
            {
                printf("Invalid format\n");
                continue;
            }

            time_t target = parse_date(date_str);
            if(target == -1) {
                printf("Invalid date format. Use dd:mm:yyyy\n");
                continue;
            }

            time_t now = time(NULL);
            double diff = difftime(now, target);

            if(strcmp(flag, "-s") == 0)
            {
                printf("%.0f seconds\n", diff);
            }
            else if(strcmp(flag, "-m") == 0)
            {
                printf("%.0f minutes\n", diff / 60);
            }
            else if(strcmp(flag, "-h") == 0)
            {
                printf("%.0f hours\n", diff / 3600);
            }
            else if(strcmp(flag, "-y") == 0)
            {
                printf("%.1f years\n", diff / (3600 * 24 * 365));
            }
            else
            {
                printf("Invalid flag\n");
            }
            session->request_count++;
        }
        else if(strncmp(command, "Sanctions", 9) == 0)
        {
            char username[7];
            int number;
            char temp_number[13];

            printf("%s\n", command);
            if(sscanf(command, "Sanctions %6s %13s", username, temp_number) != 2) {
                printf("Invalid command format\n");
                continue;
            }

            int result_string_to_number = string_to_int(temp_number, &number);
            if (result_string_to_number)
            {
                printf("Number is too long\n");
                continue;
            }

            char confirmation[20];
            printf("Enter confirmation code: ");
            scanf("%s", confirmation);
            clear_input_buffer();

            if(strcmp(confirmation, "12345") == 0)
            {
                int result = update_sanctions(username, number);
                if (result != 0)
                {
                    printf("Failed to open file\n");
                    continue;
                }
                printf("Sanctions updated!\n");
            }
            else
            {
                printf("Invalid confirmation code\n");
            }
        }
        else if (strcmp(command, "Logout") == 0)
        {
            printf("Logging out...\n");
            session->request_count = 0;
            return;
        }
        else
        {
            printf("Unknown command\n");
        }
    }

}

int main()
{

    char choice;
    while(1)
    {
        menu_authorization();
        scanf(" %c", &choice);
        switch (choice)
        {
            case '1':
            {
                char login[7];
                int pin;
                char temp_pin[13];
                printf("Enter login (up to 6 characters): ");
                scanf("%7s", login);

                clear_input_buffer();
                login[strcspn(login, "\n")] = 0;

                if (strlen(login) > 6)
                {
                    printf("Login is too long\n");
                    continue;
                }
                if (!is_valid_login(login))
                {
                    printf("Login is not valid\n");
                    continue;
                }
                printf("Enter PIN: ");
                scanf("%13s", temp_pin);
                clear_input_buffer();

                int result = string_to_int(temp_pin, &pin);

                if (result)
                {
                    printf("Pin is too long\n");
                    continue;
                }

                int flag = sign_up(login, pin);
                if (flag == 1)
                {
                    printf("Unable to open file.\n");
                    return 1;
                }
                else if (flag == 2)
                {
                    printf("Login is already taken. Choose another login.\n");
                }
                else
                {
                    printf("User registered successfully.\n");
                }
                break;
            }
            case '2':
            {
                User current;
                char login[7];
                int pin;
                char temp_pin[13];
                printf("Enter login (up to 6 characters): ");
                scanf("%7s", login);
                clear_input_buffer();
                printf("Enter PIN: ");

                scanf("%13s", temp_pin);
                clear_input_buffer();

                int result = string_to_int(temp_pin, &pin);
                if (result)
                {
                    printf("Invalid PIN\n");
                    continue;
                }
                int flag = log_in(login, pin, &current);


                if (flag == 1)
                {
                    printf("Invalid login or PIN.\n");
                }
                else
                {
                    printf("Login successful.\n");
                    Session session;
                    session.user = current;
                    session.request_count = 0;
                    sessions(&session);
                }
                break;
            }
            case '3':
            {
                printf("Exiting...\n");
                return 0;
            }
            default:
            {
                printf("Invalid choice. Try again.\n");
                break;
            }
        }
    }


}

