#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zconf.h>
#include <sys/wait.h>

void write_contents(char* path){
    switch(fork()) {
        case 0: {
            printf("%s | PID: %d\n", path, getpid());
            execl("/bin/ls", "ls", "-l", path, (char *) NULL);
        }
        default: {
            wait(0);
        }
    }

    DIR* dir = opendir(path);
    struct dirent *d;
    while ((d = readdir(dir)) != NULL) {
        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0) {
            continue;
        }
        if (d->d_type == DT_DIR) {
            char *new_path = calloc(PATH_MAX, sizeof(char));
            snprintf(new_path, PATH_MAX, "%s/%s", path, d->d_name);
            write_contents(new_path);
            free(new_path);
        }
    }
    closedir(dir);
}

int main(int argc, char** argv) {
    char *path = calloc(PATH_MAX, sizeof(char));
    getcwd(path, PATH_MAX);
    for (int i = 1; i < argc; ++i) {
        strcpy(path, argv[i]);
        if(path[strlen(path) - 1] == '/') path[strlen(path) - 1] = '\00';
    }
    write_contents(path);
}