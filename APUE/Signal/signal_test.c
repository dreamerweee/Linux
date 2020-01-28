#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void SignalHandler(int sig_num)
{
    printf("\n enter SignalHandler:\n");
    switch (sig_num) {
        case SIGINT :
            printf("get SIGINT\n");
            break;
        case SIGUSR1 :
            printf("get SIGUSR1\n");
            break;
        default :
            printf("get sig_num[%d]\n", sig_num);
            break;
    }
    // sleep(2);
}

int main()
{
    printf("begin test signal pid[%d]\n", getpid());
    if (SIG_ERR == signal(SIGINT, SignalHandler)) {
        printf("signal error\n");
        return -1;
    }
    signal(SIGUSR1, SignalHandler);
    signal(SIGQUIT, SignalHandler);
    signal(SIGTSTP, SignalHandler);
    
    printf("begin loop: \n");

    while (1) {
        write(STDOUT_FILENO, ".", 1);
        sleep(20);
    }

    return 0;
}