#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void PrintSigset(const sigset_t *set)
{
    int i;
    for (i = 1; i <= 64; ++i) {
        if (i == 33) {
            putchar(' ');
        }
        if (sigismember(set, i)) {
            putchar('1');
        } else {
            putchar('0');
        }
    }
    puts("");
}

void SignalHandler(int sig_num)
{
    if (SIGINT == sig_num) {
        printf("hello SIGINT\n");
    } else if (SIGTSTP == sig_num) {
        printf("hello SIGTSTP\n");
    }
    sleep(5);
    sigset_t st;
    sigpending(&st);
    PrintSigset(&st);
}

int main()
{
    printf("My PID[%d]\n", getpid());

    struct sigaction now_act, old_act;
    now_act.sa_handler = SignalHandler;
    sigemptyset(&now_act.sa_mask);
    sigaddset(&now_act.sa_mask, SIGINT);
    now_act.sa_flags = 0;

    sigaction(SIGINT, &now_act, &old_act);
    sigaction(SIGTSTP, &now_act, &old_act);

    while (1) {
        sleep(10);
        write(STDOUT_FILENO, ".", 1);
    }
    return 0;
}
