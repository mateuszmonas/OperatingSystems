#include <stdio.h>
#include <stdlib.h>
#include <bits/types/FILE.h>
#include <zconf.h>
#include <string.h>
#include <sys/wait.h>

int ARGMAX = 10;

void execute_line(char* line){
    int arg_count = 0;
    char **arguments = calloc(ARGMAX, sizeof(char *));
    arguments[arg_count++] = strtok(line, "|");
    while ((arguments[arg_count++] = strtok(NULL, "|"))) {}
    arg_count--;
    int pipes[2][2];
    pipe(pipes[0]);
    int pid = fork();
    if (pid == 0) {
        close(pipes[0][0]);
        dup2(pipes[0][1], STDOUT_FILENO);
        execl("/bin/sh", "sh", "-c", arguments[0], NULL);
    }
    wait(0);
    for (int i = 1; i < arg_count - 1; ++i) {
        pipe(pipes[i % 2]);
        if (fork() == 0) {
            close(pipes[(i - 1) % 2][1]);
            close(pipes[i % 2][0]);
            dup2(pipes[(i - 1) % 2][0], STDIN_FILENO);
            dup2(pipes[i % 2][1], STDOUT_FILENO);
            execl("/bin/sh", "sh", "-c", arguments[i], NULL);
        }
        close(pipes[(i - 1) % 2][0]);
        close(pipes[(i - 1) % 2][1]);
        close(pipes[i % 2][1]);
        wait(0);
    }
    if (fork() == 0) {
        close(pipes[arg_count % 2][1]);
        dup2(pipes[arg_count % 2][0], STDIN_FILENO);
        execl("/bin/sh", "sh", "-c", arguments[arg_count - 1], NULL);
    }
    close(pipes[arg_count % 2][1]);
    wait(0);
}

int main(int argc, char **argv){
    if (argc < 2) {
        fprintf(stderr, "not enough arguments\n");
        return 1;
    }
    char *filename = argv[1];

    FILE *file = fopen(filename, "r");
    char *line = calloc(2048, sizeof(char));

    while (fgets(line, 2048, file)) {
        execute_line(line);
    }

    free(line);
    return 0;
}