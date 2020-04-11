#include <stdio.h>
#include <stdlib.h>
#include <bits/types/FILE.h>



int main(int argc, char **argv){
    char *filename = "test.txt";

    FILE *file = fopen(filename, "r");
    char *line = calloc(2048, sizeof(char));

    while (fgets(line, 2048, file)) {

    }

    free(line);
    return 0;
}