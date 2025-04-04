#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#define MAX_PATH 4096

// Функция определяет тип сущности на основе stat()
const char* get_file_type(const char *full_path) {
    struct stat file_stat;
    if (stat(full_path, &file_stat) == 0) {
        if (S_ISREG(file_stat.st_mode))  return "File";
        if (S_ISDIR(file_stat.st_mode))  return "Directory";
        if (S_ISLNK(file_stat.st_mode))  return "Symbolic Link";
        if (S_ISFIFO(file_stat.st_mode)) return "FIFO (Named Pipe)";
        if (S_ISSOCK(file_stat.st_mode)) return "Socket";
        if (S_ISBLK(file_stat.st_mode))  return "Block Device";
        if (S_ISCHR(file_stat.st_mode))  return "Character Device";
    }
    return "Unknown";
}

void list_directory(const char *path) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    struct dirent *entry;
    char full_path[MAX_PATH];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        // Получаем тип сущности
        const char *type = get_file_type(full_path);

        printf("Inode: %-10lu Type: %-15s Name: %s\n", entry->d_ino, type, entry->d_name);
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <directory> [directory ...]\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        printf("\nDirectory: %s\n", argv[i]);
        list_directory(argv[i]);
    }

    return 0;
}
