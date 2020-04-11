#include <stdlib.h>
#include <stdio.h>
#include <zconf.h>

int main(int argc, char **argv){
    char *pipe_name = argv[1];
    char *file_name = argv[2];
    int N = (int) strtol(argv[3], NULL, 10);
    char *line = calloc(N, sizeof(char));
    FILE* pipe = fopen(pipe_name, "w");
    FILE* file = fopen(file_name, "r");

    while (fgets(line, N, file)) {
        fprintf(pipe, "#%d#%s\n", getpid(), line);
        sleep(1);
    }

    fclose(file);
    fclose(pipe);
    free(line);
    return 0;
}