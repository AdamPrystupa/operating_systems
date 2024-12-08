#include <stdio.h>
#include <signal.h>
#include <time.h>

volatile sig_atomic_t signals_enable= 1; 
volatile sig_atomic_t periodic = 0;

void signals_handling(int sig) {
    switch(sig) {
        default:
            case SIGALRM:
            case SIGTERM:
                printf("Otrzymano sygnal SIGTERM: zamykanie programu.\n");
                _exit(0);
                break;
            case SIGUSR1:
                printf("Otrzymano sygnal SIGUSR1: kontynuacja programu.\n");
                break;
            case SIGUSR2:
                printf("Otrzymano sygnal SIGUSR2: odbieranie sygnalu tylko w krotkich okresach.\n");
                signals_enable = 0;
                periodic = 1;
                break;

}
}

int main() {
    int i = 0;
    struct timespec ts = {0,  10000000};

    signal(SIGALRM, signals_handling);
    signal(SIGTERM, signals_handling);
    signal(SIGUSR1, signals_handling);
    signal(SIGUSR2, signals_handling);

    while (1) {
        ++i;
        if (periodic==1 && i % 1000 == 0) {
            printf("Odbieranie sygnalow aktywne.\n");
            signals_enable=1;
            nanosleep(&ts, NULL);
            printf("Odbieranie sygnalow nieaktywne.\n");
            signals_enable=0;
        }
        nanosleep(&ts, NULL);
    }
    return 0;
}
