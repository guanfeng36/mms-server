#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <execinfo.h>
#include <unistd.h>

#include "common/climit.h"
#include "common/common.h"

void segv_handle(int signum) {
    signal(SIGSEGV, SIG_DFL);

    void *array[10];
    size_t size;
    char **strings;
    size_t i;

    signal(signum, SIG_DFL);

    size = backtrace (array, 10);
    strings = (char **)backtrace_symbols (array, size);

    fprintf(stderr, "widebright received SIGSEGV! Stack trace:\n");
    for (i = 0; i < size; i++) {
        fprintf(stderr, "%d %s \n", (int)i, strings[i]);
    }

    free (strings);
}

void add_sigsegv_handler() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = segv_handle;
    sigemptyset(&sa.sa_mask);
    if(sigaction(SIGSEGV, &sa, 0) < 0) {
        perror("sigaction");
        exit(1);
    }
}

int get_process_name(char* name, int max_len) {
    char* path_end; 
    char processdir[FILE_NAME_LEN_MAX] = {0};

    int len = readlink("/proc/self/exe", processdir, sizeof(processdir) - 1);
    if(len <=0) return -1; 
    path_end = strrchr(processdir,  '/'); 
    if(path_end == NULL)  return -1; 

    ++path_end; 
    strncpy(name, path_end, max_len);
    name[max_len - 1] = '\0';
    return (len - (path_end - processdir));
}
