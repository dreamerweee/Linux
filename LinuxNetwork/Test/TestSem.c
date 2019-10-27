#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
};

// op为-1时执行p操作，op为1时执行v操作
void PV(int sem_id, int op)
{
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = op;
	sem_b.sem_flg = SEM_UNDO;
	semop(sem_id, &sem_b, 1);
}

int main(int argc, char *argv[])
{
	int sem_id = semget(IPC_PRIVATE, 1, 0666);

	union semun sem_un;
	sem_un.val = 1;
	semctl(sem_id, 0, SETVAL, sem_un);

	pid_t pid = fork();
	if (pid < 0) {
		exit(-1);
	}

	if (pid == 0) {
		printf("child try to get binary sem\n");
		// 在父、子进程间共享IPC_PRIVATE信号量的关键就在于二者都可以操作该信号量的标识符sem_id
		PV(sem_id, -1);
		printf("child get the sem and would release it after 5s\n");
		sleep(5);
		PV(sem_id, 1);
		exit(0);
	} else {
		printf("parent try to get binary sem\n");
		PV(sem_id, -1);
		printf("parent get the sem and wold release it after 5s\n");
		sleep(5);
		PV(sem_id, 1);
	}

	waitpid(pid, NULL, 0);
	semctl(sem_id, 0, IPC_RMID, sem_un);  // 删除信号量
	return 0;
}
