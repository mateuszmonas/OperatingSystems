#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv){
    char *pipe_name = argv[1];
    char *file_name = argv[2];
    int N = (int) strtol(argv[3], NULL, 10);
    char *line = calloc(N, sizeof(char));
    FILE* pipe = fopen(pipe_name, "r");
    FILE* file = fopen(file_name, "w");

    while (fgets(line, N, pipe)) {
        fprintf(file, "%s", line);
    }
    fclose(file);
    fclose(pipe);
    free(line);
    return 0;
}