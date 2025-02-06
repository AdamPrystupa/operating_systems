#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void TERM_handle(int sig)
{
    printf("Otrzymano sygnal SIGTERM: zamykanie programu.\n");
    _exit(0);
};

void USR1_handle(int sig)
{
    printf("Otrzymano sygnal SIGUSR1: kontynuacja programu.\n");
}

void USR2_handle(int sig)
{
    printf("Otrzymano sygnal SIGUSR2.\n");
}

int main()
{
    int i = 0;
    struct timespec ts = {0, 10000000};
    sigset_t sigset;

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR2);
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    signal(SIGALRM, SIG_IGN);
    signal(SIGTERM, TERM_handle);
    signal(SIGUSR1, USR1_handle);
    signal(SIGUSR2, USR2_handle);

    while (1)
    {
        ++i;
        if (i % 1000 == 0)
        {
            printf("Odblokowano odbieranie sygnalu!.\n");
            sigprocmask(SIG_UNBLOCK, &sigset, NULL);
            nanosleep(&ts, NULL);
            sigprocmask(SIG_BLOCK, &sigset, NULL);
            printf("Zablokowano odbieranie sygnalu!.\n");
        }
        nanosleep(&ts, NULL);
    }
    return 0;
}
