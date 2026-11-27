// runm: runs a command with its output to /dev/null; prints wall seconds, peak RSS (KB),
// exit status, and CPU seconds (user + system) of the child
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/resource.h>
int main (int argc, char** argv) {
    struct timespec a, b; clock_gettime(CLOCK_MONOTONIC, &a);
    pid_t p = fork();
    if (p == 0) { int fd = open("/dev/null", O_WRONLY); dup2(fd, 1); execvp(argv[1], argv+1); _exit(127); }
    int st; struct rusage ru; wait4(p, &st, 0, &ru);
    clock_gettime(CLOCK_MONOTONIC, &b);
    double cpu = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec/1e6 + ru.ru_stime.tv_sec + ru.ru_stime.tv_usec/1e6;
    printf("%.3f %ld %d %.3f\n", (b.tv_sec-a.tv_sec) + (b.tv_nsec-a.tv_nsec)/1e9, ru.ru_maxrss, WEXITSTATUS(st), cpu);
    return 0;
}
