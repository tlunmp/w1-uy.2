#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>	
#include <sys/shm.h>

#define SHMKEY 9785

int shmid;
int *shmPtr;

int main (int argc, char *argv[]) {	

	int masterShare = 0;	
	int arr[2];
	
	shmid = shmget(SHMKEY, sizeof(arr[2]), 0777|IPC_CREAT);

	if(shmid < 0) {
		perror("Error: shmget worker");
		exit(errno);
	}

	shmPtr = (int *)(shmat(shmid, NULL, 0));
	
	masterShare = shmPtr[0];
	printf(" master %d\n",masterShare);			

	return 0;
}
