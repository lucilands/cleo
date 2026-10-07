#include <sys/prctl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern char **environ;
static char *title_start, *title_end;

char *strdup(char *a) {
    char *ret = malloc(strlen(a) + 1);
    if (!ret) return NULL;
    memset(ret, 0, strlen(a)+1);

    strcpy(ret, a);
    return ret;
}

void proctitle_init(int argc, char **argv) {
    title_start = argv[0];
    title_end   = argv[argc - 1] + strlen(argv[argc - 1]) + 1;

    int n = 0;
    for (; environ[n]; n++)
        if (environ[n] == title_end)
            title_end += strlen(environ[n]) + 1;

    char **newenv = malloc((n + 1) * sizeof *newenv);
    for (int i = 0; i < n; i++)
        newenv[i] = strdup(environ[i]);
    newenv[n] = NULL;
    environ = newenv;
}

void proctitle_set(const char *title) {
    size_t avail = title_end - title_start;
    memset(title_start, 0, avail);
    strncpy(title_start, title, avail - 1);

    prctl(PR_SET_NAME, title);
    printf("\033]0;%s\007", title);
    fflush(stdout);
}
