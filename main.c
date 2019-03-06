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

void helpMenu();
void forkProcess(int maxChildProcess, int numberChildProcess,char *inputFileName, char *outputFileName, int increment);
void shareClock(int increment);
int countLines(FILE *file);

void signalCall(int signum);



int timer = 10;
int shmid;
int *shmPtr;

struct Clock {
	int seconds;
	int nanoseconds;
	int duration;
};

int main (int argc, char *argv[]) {

	char inputFileName[] = "input.txt";
	char outputFileName[] = "output.txt";
	int bufSize = 100;
	char buffer[bufSize];
		
	int newLineCount = 0;
	int c;
	int maxChildProcess = 0;
	int numberChildProcess = 0;

	

	//getopt command for command line
	while((c = getopt (argc,argv, "hi:o:n:s:")) != -1) {

		switch(c) {
			case 'h':
				helpMenu();
				return 1;
			case 'i':
				strcpy(inputFileName, optarg);
				break;
			case 'o':
				strcpy(outputFileName, optarg);
				break;
			case 'n': maxChildProcess = atoi(optarg);
				break;
			case 's':numberChildProcess = atoi(optarg);
				break;
			default:
				fprintf(stderr, "%s: Error: Unknown option -%c\n",argv[0],optopt);
				return -1;	
		}


	}

	FILE *f = fopen(inputFileName,"r");
	
	// if file open error and return
	if(f == NULL){
		fprintf(stderr,"%s: ", argv[0]);
		perror("Error");
		return 0;
	}


	//gets the fork number
	fgets(buffer, bufSize, f);
		newLineCount++;
	fclose(f);

	int increment = atoi(buffer);
	int secClock = 0, nanoClock = 0;	
	char *args[3];
	int i, newLineCompare=0;
	
	FILE *f1 = fopen(inputFileName, "r");

	 if (signal(SIGINT, signalCall) == SIG_ERR) {
        	perror("Error: worker: signal(): SIGINT\n");
        	exit(errno);
  	  }
	
	if (signal(SIGALRM, signalCall) == SIG_ERR) {
         perror("Error: worker: signal(): SIGALRM\n");
         exit(errno);
     	}
	
	alarm(timer);
	forkProcess(maxChildProcess, numberChildProcess,inputFileName,outputFileName,increment);
		
		
	return 0;
}	


void forkProcess(int maxChildProcess, int numChildProcess, char *inputFileName, char *outputFileName,int increment) {
		
	int status, arr[2];
	char * args[2];

	int bufSize = 100;
	char buffer[bufSize];
	
	int newLineCount = 1;
	int newLineCompare = 0;

	int secClock = 0;
	int nanoClock = 0;
	
	pid_t childpid;
	int ptr_count = 0;


	int maxChildCount = 0;


	

	if((shmid = shmget(SHMKEY, sizeof(arr[2]), 0777 | IPC_CREAT )) < 0){
           	printf("shmget failed in master\n");	
		exit(1);	
  	}
            	   	        
	char * shmaddr = ( char * )( shmat ( shmid, NULL, 0 ) );
        shmPtr = ( int * )( shmaddr);
            	   	                            
	if(shmPtr == -1 ){
            	printf("shmat failed in master");
            	exit(2);	
        }

	shmPtr[0] = 0; 
	shmPtr[1] = 0; 

	int flag = 0;
		

	
	FILE *f1 = fopen(inputFileName,"r");

	int  line = countLines(f1)-1;

	int stackSize = line *3;
	

	
	int i,m=0;
	

	struct Clock clock[stackSize];
 
	while(fgets(buffer,bufSize,f1)!= 0){

		if(newLineCount == newLineCompare) {

			//parsing the text
			for(i =0; i < 3; i++)
	       		{
				args[i] = (char*) malloc(3);

	       		}

			//extracting the commands
	       		sscanf(buffer, "%s %s %s", args[0], args[1],args[2]);
	
			clock[m].seconds = atoi(args[0]);
			clock[m].nanoseconds = atoi(args[1]);	
			clock[m].duration = atoi(args[2]);	
			m++;
			newLineCount++;
		}

		newLineCompare++;
	}
	fclose(f1);
		

	int s = 0;
	int totalCount = 0;


	while( totalCount < maxChildProcess || ptr_count > 0){
		
		shmPtr[1] += increment;

		if(shmPtr[1] > 1000000000){
			shmPtr[0]++;
			shmPtr[1] -= 1000000000;
		}				
		

			if(waitpid(0, NULL, WNOHANG) > 0){
				ptr_count--;
			}

		if( ptr_count < numChildProcess && shmPtr[0] == clock[s].seconds && shmPtr[1] > clock[s].nanoseconds){		
			childpid = fork();
			totalCount++;
			ptr_count++;

					
			if(childpid == 0){
				char duration[100];
				sprintf(duration, "%d", clock[s].duration);
				execl("./user","user",duration,outputFileName,(char *)0);
				perror("exec didnt work");
				exit(0);
			
			} else {
				FILE *f3 = fopen(outputFileName,"a");
			fprintf(f3,"PID: %d, Launch Time: %d Seconds %d Nanoseconds\n",childpid,shmPtr[0], shmPtr[1]);
		//printf("%d, parent main start launch %d and %d\n",getpid(),shmPtr[0], shmPtr[1]);
				fclose(f3);
			}
			s++;
		}	

	}
	
	    shmdt(shmPtr); //detaches a section of shared memory
    	shmctl(shmid, IPC_RMID, NULL);  // deallocate the memory    
		
 }



void signalCall(int signum)
{
    int status;
  //  kill(0, SIGTERM);
    if (signum == SIGINT)
        printf("\nSIGINT received by master\n");
    else
        printf("\nSIGALRM received by master\n");
 
    while(wait(&status) > 0) {
        if (WIFEXITED(status))  /* process exited normally */
                printf("User process exited with value %d\n", WEXITSTATUS(status));
        else if (WIFSIGNALED(status))   /* child exited on a signal */
                printf("User process exited due to signal %d\n", WTERMSIG(status));
        else if (WIFSTOPPED(status))    /* child was stopped */
                printf("wUser process was stopped by signal %d\n", WIFSTOPPED(status));
    }
    kill(0, SIGTERM);
    //clean up program before exit (via interrupt signal)
    shmdt(shmPtr); //detaches a section of shared memory
    shmctl(shmid, IPC_RMID, NULL);  // deallocate the memory
   
      exit(EXIT_SUCCESS);
 }

  
   
int countLines(FILE *file) {
  int lines = 0;
  char c;
  char last = '\n';
  while (EOF != (c = fgetc(file))) {
    if (c == '\n' && last != '\n') {
      ++lines;
    }
    last = c;
  }
  /* Reset the file pointer to the start of the file */
  rewind(file);
  return lines;
}

void shareClock(int increment){

	int i,j;
	pid_t childpid;


	
		for(;;) {
	//			printf("%d and %d\n",shmPtr[0],shmPtr[1]);
	//		if(secClockSet == shmPtr[0] && nanoClockSet > shmPtr[1]) {
	//			//childpid = fork();
				break;
	//		}

			if(shmPtr[1] > 1000000000) {
				shmPtr[0]++;
				shmPtr[1] = 0;
			}

		
			shmPtr[1] += 20000;
	
		}



//	printf("%d and %d\n",secClockSet,nanoClockSet);

}
//help menu
void helpMenu() {
		printf("---------------------------------------------------------------| Help Menu |--------------------------------------------------------------------------\n");
		printf("-h help menu\n"); 
		printf("-i inputfilename                      | inputfilename is where the filename reads and it will show error if there is no filename found on the directory.\n"); 
		printf("                                      | output filename should be default name is output.dat  where the result is generated.\n");
		printf("-o outputfilename                     | this command will use the default input file which is input.dat\n"); 
		printf("  				      | then create an output result to outputfilename(Which is the user specified name of the file \n"); 
		printf("-i inputfilename -o outputfilename    | this command can use inputfilename (user choose the name)\n");
		printf("				      | generate output to the outputfilename(user choose the outputname) if it doesnt exist create one.\n"); 
		printf("------------------------------------------------------------------------------------------------------------------------------------------------------\n");
}

