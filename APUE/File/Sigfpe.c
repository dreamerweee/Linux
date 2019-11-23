#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void SigFpe(int signo)
{
	printf("handle signal value[%d]\n", signo);
	exit(-1);
}

int GetValue()
{
	int value;
	scanf("%d", &value);
	return value;
}

int main()
{
	signal(SIGFPE, SigFpe);
	int result = 10 / GetValue();
	return 0;
}