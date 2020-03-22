#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <stdbool.h>
#include <zconf.h>

static struct timespec program_start_time;

char* time_stamp_to_time(time_t timestamp){
    int size = 256;
    char *res = calloc(size, sizeof(char));
    struct tm local_time;
    localtime_r(&timestamp, &local_time);
    strftime(res, size, "%F %T", &local_time);
    return res;
}

char* d_type_to_string(int d_type){
    switch (d_type) {
        case DT_BLK:
            return  "block dev";
        case DT_CHR:
            return  "char device";
        case DT_DIR:
            return  "dir";
        case DT_FIFO:
            return  "fifo";
        case DT_LNK:
            return  "slink";
        case DT_REG:
            return  "file";
        case DT_SOCK:
            return  "sock";
        case DT_UNKNOWN:
        default:
            return "unknown";
    }
}

struct time_filter{
    bool modification_time_set;
    bool access_time_set;
    char* modification_time_modifier;
    char* access_time_modifier;
    long modification_time;
    long access_time;
};

bool filter_dir(struct stat* dir_stat, struct time_filter* filter){
    unsigned long mtime_diff = program_start_time.tv_sec - dir_stat->st_mtim.tv_sec;
    unsigned long atime_diff = program_start_time.tv_sec - dir_stat->st_atim.tv_sec;
    bool mod_time_match = true;
    if (filter->modification_time_set) {
        if (strcmp(filter->modification_time_modifier, "+") == 0) {
            mod_time_match = (mtime_diff / 86400) > filter->modification_time;
        } else if (strcmp(filter->modification_time_modifier, "-") == 0) {
            mod_time_match = (mtime_diff / 86400) < filter->modification_time;
        } else {
            mod_time_match = (mtime_diff / 86400) == filter->modification_time;
        }
    }
    if (mod_time_match && filter->access_time_set) {
        if (strcmp(filter->access_time_modifier, "+") == 0) {
            mod_time_match = (atime_diff / 86400) > filter->access_time;
        } else if (strcmp(filter->access_time_modifier, "-") == 0) {
            mod_time_match = (atime_diff / 86400) < filter->access_time;
        }else {
            mod_time_match = (atime_diff / 86400) == filter->access_time;
        }
    }
    return mod_time_match;
}

void write_contents(char* path, struct time_filter* filter, long max_depth){
    if (max_depth <= 0) {
        return;
    }
    DIR* dir = opendir(path);
    char *new_path = calloc(PATH_MAX, sizeof(char));
    struct stat *file_stats = calloc(1, sizeof *file_stats);
    struct dirent *d;
    while ((d = readdir(dir)) != NULL) {
        snprintf(new_path, PATH_MAX, "%s/%s", path, d->d_name);
        stat(new_path, file_stats);
        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0 || !filter_dir(file_stats, filter)) {
            continue;
        }
        char *access_time = time_stamp_to_time(file_stats->st_atim.tv_sec);
        char *modification_time = time_stamp_to_time(file_stats->st_mtim.tv_sec);
        printf("%s/%s | links: %lu | type: %s | size: %ld | access time: %s | modification time: %s\n", path, d->d_name,
               file_stats->st_nlink,
               d_type_to_string(d->d_type), file_stats->st_size,
               access_time,
               modification_time);
        free(access_time);
        free(modification_time);
        if (d->d_type == DT_DIR) {
            write_contents(new_path, filter, max_depth - 1);
        }
    }
    free(file_stats);
    free(new_path);
    closedir(dir);
}

int main(int argc, char** argv) {
    clock_gettime(CLOCK_REALTIME, &program_start_time);
    struct time_filter fltr = {false, false, "", "", 0, 0};
    char *path = calloc(PATH_MAX, sizeof(char));
    getcwd(path, PATH_MAX);
    long max_depth = LONG_MAX;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-mtime") == 0) {
            char *val = argv[++i];
            fltr.modification_time_set = true;
            fltr.modification_time_modifier = &val[0];
            fltr.modification_time = strtol(val, NULL, 10);
        }else if (strcmp(argv[i], "-atime") == 0) {
            char *val = argv[++i];
            fltr.access_time_set = true;
            fltr.access_time_modifier = &val[0];
            fltr.access_time = strtol(val, NULL, 10);
        }else if (strcmp(argv[i], "-maxdepth") == 0) {
            max_depth = strtol(argv[++i], NULL, 10);
        }else {
            strcpy(path, argv[i]);
        }
    }
    write_contents(path, &fltr, max_depth);
}