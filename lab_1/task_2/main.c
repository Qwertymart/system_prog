#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdint.h>
#include "limits.h"

//под винду
//для пингвинчика 4096
#define FILENAME 272
#define COPYMAX 10

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

int xor2(FILE* file, unsigned char* result) {
    *result = 0;
    unsigned char buffer;
    while (fread(&buffer, 1, 1, file) > 0) {
        *result ^= (buffer & 0x0F) ^ ((buffer >> 4) & 0x0F); // байт делим на 2 части
    }
    return 0;
}

int xor3(FILE* file, unsigned char* result) {
    *result = 0;
    unsigned char buffer;
    while (fread(&buffer, 1, 1, file) > 0) {
        *result ^= buffer;
    }
    return 0;
}

int xor4(FILE* file, unsigned char* result) {
    *result = 0;
    unsigned int buffer = 0;
    while (fread(&buffer, 1, 2, file) > 0) {
        *result ^= (buffer & 0xFF) ^ ((buffer >> 8) & 0xFF);
    }
    return 0;
}

int xor5(FILE* file, unsigned char* result) {
    *result = 0;
    unsigned int buffer = 0;
    while (fread(&buffer, 1, 4, file) > 0) {
        *result ^= (buffer & 0xFF) ^ ((buffer >> 8) & 0xFF) ^ ((buffer >> 16) & 0xFF) ^ ((buffer >> 24) & 0xFF);
    }
    return 0;
}

int xor6(FILE* file, unsigned char* result) {
    *result = 0;
    unsigned char buffer[8] = {0};
    while (fread(buffer, 1, 8, file) > 0) {
        for (int i = 0; i < 8; i++) {
            *result ^= buffer[i];
        }
    }
    return 0;
}


int count_mask_numbers(FILE* file, uint32_t mask)
{
    uint32_t number;
    int count = 0;

    while(fread(&number, sizeof(uint32_t), 1, file) == 1)
    {
        if((number & mask) == mask)
        {
            ++count;
        }
    }
    return count;
}


int copy_file(const char* filename, char* dest)
{
    FILE* source = fopen(filename, "rb");
    if (!source)
    {
        return 1;
    }

    FILE* destination = fopen(dest, "wb");
    if (!destination)
    {
        fclose(source);
        return 1;
    }

    char buffer[1024];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), source)) > 0)
    {
        fwrite(buffer, 1, bytes, destination);
    }
    fclose(source);
    fclose(destination);
    return 0;
}


int create_copies(const char* filename, int count_copies)
{
    for(int i = 1; i <= count_copies; ++i)
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            return 1;
        }
        if (pid == 0)
        {
            char dest[FILENAME];
            snprintf(dest, sizeof(dest), "%s_%d", filename, i);
            if (copy_file(filename, dest))
            {
                return 2;
            }
            exit(0);
        }
    }
    for (int i = 0; i < count_copies; i++)
    {
        wait(NULL);
    }

    return 0;
}


int find_process( const char* string, const char* filename, char* match)
{
    FILE* file = fopen(filename, "r");
    if (!file)
    {
        return 1;
    }
    *match = 0;
    size_t i = 0;
    char ch;
    while ((ch = fgetc(file)) != EOF)
    {
        if (ch == string[i])
        {
            ++i;
            if (string[i] == '\0')
            {
                fclose(file);
                *match = 1;
                return 0;
            }
        }
        else
        {
            i = 0;
        }
    }
    fclose(file);
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf( "Usage: %s <files> <flag>\n", argv[0]);
        return 1;
    }

    char* flag_1 = argv[argc - 1];
    char* flag_2 = argv[argc - 2];
    int N = 0;
    if (strncmp(flag_1, "xor", sizeof(char) * 3) == 0)
    {
        if (sscanf(flag_1 + 3, "%d", &N) != 1 || N < 2 || N > 6)
        {
            printf( "Invalid value for N. Must be between 2 and 6.\n");
            return 1;
        }

        for(int i = 1; i < argc - 1; ++i)
        {
            FILE* file = fopen(argv[i], "rb");
            if (!file)
            {
                printf("Error opening file\n");
                return 2;
            }
            unsigned char result;
            if (N == 2)
            {
                xor2(file, &result);
            }
            else if( N == 3)
            {
                xor3(file, &result);
            }
            else if( N == 4)
            {
                xor4(file, &result);
            }
            else if( N == 5)
            {
                xor5(file, &result);
            }
            else if( N == 6)
            {
                xor6(file, &result);
            }
            else
            {
                printf("pisdik\n");
                return 1;
            };
            printf("%d\n", result);
            fclose(file);
        }

    }
    else if(strcmp(flag_2, "mask") == 0)
    {
        uint32_t mask;
        if (sscanf(flag_1, "%x", &mask) != 1)
        {
            printf("Invalid mask\n");
            return 1;
        }

        int result = 0;
        for(int i = 1; i < argc - 2; ++i)
        {
            FILE *file = fopen(argv[i], "rb");
            if (!file) {
                printf("Error opening file\n");
                return 2;
            }
            result += count_mask_numbers( file, mask);
            fclose(file);
        }

        printf("Total count: %d\n", result);
    }


    else if(strncmp(flag_1, "copy", sizeof(char) * 4) == 0)
    {
        char n[15];
        int count_copies = 0;
        if (sscanf(flag_1 + 4, "%15s", n))
        {
            if(string_to_int(n, &count_copies))
            {
                printf("Input error\n");
                return 1;
            }
            if (count_copies > COPYMAX)
            {
                printf("Input error\n");
                return 1;
            }
            for(int i = 1; i < argc - 1; ++i)
            {
                int result = create_copies(argv[i], count_copies);
                if (result == 1)
                {
                    printf("Fork failed\n");
                    return 5;
                }
                if (result == 2)
                {
                    printf("Error opening file\n");
                    return 2;
                }
            }

        }
        else
        {
            printf("Input error\n");
            return 1;
        }
    }
    else if (strcmp(flag_2, "find") == 0)
    {
        const char *search_string = argv[argc - 1];
        int found = 0;

        for (int i = 1; i < argc - 2; ++i) {
            pid_t pid = fork();
            if (pid == -1) {
                perror("Fork failed");
                return 2;
            }

            if (pid == 0) {
                char match = 0;
                int result = find_process(search_string, argv[i], &match);
                if (result)
                {
                    printf("Error opening file\n");
                    return 3;
                }
                if (match)
                {
                    printf("Found in: %s\n", argv[i]);
                    break;
                }
                else
                {
                    printf("Not found in: %s\n", argv[i]);
                    break;
                }
            }
        }

        for (int i = 1; i < argc - 2; ++i) {
            wait(NULL);
        }

    }
    else
    {
            printf("Invalid command\n");
            return 1;
    }
}
