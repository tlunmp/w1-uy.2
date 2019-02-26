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
void signalCallback (int signum);

int main (int argc, char *argv[]) {	

	int masterShare = 0;	
	int arr[2];
	
	shmid = shmget(SHMKEY, sizeof(arr[2]), 0666|IPC_CREAT);

	
	if(shmid < 0) {
		perror("Error: shmget worker");
		exit(errno);
	}

	shmPtr = (int *)(shmat(shmid, NULL, 0));
	
	masterShare = shmPtr[0];
	int masterShare2 = shmPtr[1];
	

	
	printf("share 1 %d\n",masterShare2);	
	int duration = atoi(argv[1]);
	
	masterShare2 += duration;
	
	printf("share %d\n",masterShare2);	
	
	shmdt(shmPtr);
	shmctl(shmid, IPC_RMID, NULL);

	return 0;
}

void signalCallback (int signum)
{
    printf("\nSIGTERM received by worker\n");

    //Cleanup
        shmdt(shmPtr);
            shmctl(shmid,IPC_RMID, NULL);
                exit(0);
               }
 
