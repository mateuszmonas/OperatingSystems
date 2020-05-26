#include <stdio.h>
#include <stdlib.h>
#include <bits/types/FILE.h>



int main(int argc, char **argv){
    if (argc < 2) {
        fprintf(stderr, "not enough arguments\n");
        return 1;
    }
    char *filename = argv[1];
    FILE *file = fopen(filename, "r");
    char *command = calloc(2048, sizeof(char));
    snprintf(command, 2048, "sort %s", filename);
    FILE *sort_out = popen(command, "r");
    char *line = calloc(2048, sizeof(char));
    while (fgets(line, 2048, sort_out)) {
        printf("%s", line);
    }
    pclose(sort_out);
    fclose(file);
    free(line);
    free(command);
    return 0;
}