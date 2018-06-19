#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static int search(const char *argument, const char *root, const char *path) {

    DIR *directory_pointer;
    struct dirent *dp;
    int found = 0;
    if ((directory_pointer = opendir(path)) == NULL) {
        perror("Can't open directory\n\n");
        return found;
    }
    do {
        errno = 0;
        if ((dp = readdir(directory_pointer)) != NULL) {
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
                continue;
            if (strcmp(dp->d_name, argument) == 0) {
                found = 1;
                struct stat information;
                char directory_path[1000];
                memset(directory_path, 0, sizeof (directory_path));
                strcat(directory_path, path);
                strcat(directory_path, "/");
                strcat(directory_path, dp->d_name);
                stat(directory_path, &information);
                (void) printf("File found: %s\nType: %d\nSize: %ld bytes\nPhisycal size: %ld bytes\nNumber of blocks: %ld\nBlock size: %ld\n", directory_path, dp->d_type, information.st_size, information.st_blocks * 512, information.st_blocks, information.st_blksize);
                (void) closedir(directory_pointer);
                return found;
            }
            if (dp->d_type == DT_DIR) {
                char directory_path[1000];
                memset(directory_path, 0, sizeof (directory_path));

                strcat(directory_path, path);
                strcat(directory_path, "/");
                strcat(directory_path, dp->d_name);
                (void) printf("%s\n", directory_path);
                if (search(argument, root, directory_path) == 1) {
                    return 1;
                }
            }
            if (strcmp(dp->d_name, argument) != 0)
                continue;
        }
        if (found == 0) {
            (void) closedir(directory_pointer);
            return found;
        }
    } while (dp != NULL);

    if (errno != 0)
        perror("Error reading directory\n\n");
    else
        (void) printf("Fail while searching: %s\n", argument);
    (void) closedir(directory_pointer);
    return found;
}

int main(int argc, char *argv[]) {
    int i;

    if (argc < 2) {
        printf("USE: %s file_name...\n\n", argv[ 0 ]);
        return ( -1);
    }
    for (i = 1; i < argc; i++)
        if (!search(argv[i], ".", "."))
            printf("Nothing Found\n");
    return (0);
}