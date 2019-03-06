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

	int shareSeconds;
	int shareNanoSeconds;	
	int arr[2];
	char errorMessage[1000];	
	shmid = shmget(SHMKEY, sizeof(arr[2]), 0666|IPC_CREAT);

	
	if(shmid < 0) {
		perror("Error: shmget worker");
		exit(errno);
	}

	shmPtr = (int *)(shmat(shmid, NULL, 0));
	
	shareSeconds = shmPtr[0];
	shareNanoSeconds = shmPtr[1];

	//printf("%d, main start launch %d and %d\n",getpid(),shmPtr[0], shmPtr[1]);
	//printf("this first  %d and %d\n", shareSeconds, shareNanoSeconds);
	
	int duration = atoi(argv[1]);
	
	int endDurationSeconds = duration/(int)1E9 + shareSeconds;
	int endDurationNanoSeconds = duration%(int)1E9 + shareNanoSeconds;


	//printf("%d and %d\n", endDurationSeconds, endDurationNanoSeconds);	
	

	FILE *f1 = fopen(argv[2],"a");

	if(f1 == NULL){
		snprintf(errorMessage, sizeof(errorMessage), "%s: Error: ", argv[0]);
	     	perror(errorMessage);	
		return 0;
	}


	while(endDurationSeconds > shmPtr[0] || (endDurationSeconds == shmPtr[0] && endDurationNanoSeconds > shmPtr[1] ));
	
	fprintf(f1,"PID: %d: Termination %d Seconds %d Nanoseconds\n",getpid(),shmPtr[0], shmPtr[1]);
	
	fclose(f1);
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
 
