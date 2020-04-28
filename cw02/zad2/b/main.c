#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <stdbool.h>
#include <zconf.h>
#define __USE_XOPEN_EXTENDED 1
#include <ftw.h>

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
        case FTW_DNR:
            return  "unr dir";
        case FTW_NS:
            return  "uns file";
        case FTW_D:
            return  "dir";
        case FTW_SL:
            return  "slink";
        case FTW_F:
            return  "file";
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

struct time_filter fltr = {false, false, "", "", 0, 0};
long max_depth = LONG_MAX;

bool filter_dir(const struct stat* dir_stat, struct time_filter* filter){
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

int print_description(const char *fpath, const struct stat *sb,
                      int typeflag, struct FTW *ftwbuf){
    if(!filter_dir(sb, &fltr) || ftwbuf->level > max_depth){
        return 0;
    }
    char *access_time = time_stamp_to_time(sb->st_atim.tv_sec);
    char *modification_time = time_stamp_to_time(sb->st_mtim.tv_sec);
    printf("%s | links: %lu | type: %s | size: %ld | access time: %s | modification time: %s\n", fpath,
           sb->st_nlink,
           d_type_to_string(typeflag), sb->st_size,
           access_time,
           modification_time);
    free(access_time);
    free(modification_time);
    return 0;
}

int main(int argc, char** argv) {
    clock_gettime(CLOCK_REALTIME, &program_start_time);
    char *path = calloc(PATH_MAX, sizeof(char));
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
    nftw(path, print_description, 0, 0);
}