#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

void init(int argc, char **argv, struct stat *st, long *last_mods)
{
    int i, res;
    for(i = 1; i < argc - 1; i++) {
        res = stat(argv[i], st);
        if(res == 0)
            last_mods[i] = st->st_mtimespec.tv_sec;
        else
            perror("stat");
    }
}

void execute_script(int argc, char **argv)
{
    int pid, status;
    pid = fork();
    char* a[] = { "sh", argv[argc - 1], NULL };
    if(pid == 0) {
        execvp(a[0],  a);
        perror(argv[argc - 1]);
        exit(2);
    } else {
        wait(&status);
        if(WEXITSTATUS(status))
            printf("Error. Process has exited with code %d\n",
                WEXITSTATUS(status));
    }
}

void start_loop(int argc, char **argv, struct stat *st, long *last_mods)
{
    int i, res;
    for(i = 1; i < argc - 1; i++) {
        res = stat(argv[i], st);
        if(res != 0) {
            perror("struct stat");
            exit(1);
        }
        if(last_mods[i] != st->st_mtimespec.tv_sec) {
            last_mods[i] = st->st_mtimespec.tv_sec;
            execute_script(argc, argv);
        }    
    }
}

int main(int argc, char **argv)
{
    enum { check_time = 1, min_args = 3, max_args = 6 };
    struct stat st;
    long last_mods[max_args];

    if(argc < min_args || argc > max_args) {
        fprintf(stderr, "Parameters should be between %d and %d\n",
            min_args - 1, max_args - 1);
        return 1;
    }

    init(argc, argv, &st, last_mods);

    for(;;) {
        start_loop(argc, argv, &st, last_mods);
        sleep(check_time);
    }
    return 0;
}