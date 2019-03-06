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
void forkProcess(int maxChildProcess, int numberChildProcess,char *inputFileName, char *outputFileName, int increment, char *arg0Name);
void shareClock(int increment);
int countLines(FILE *file);

void signalCall(int signum);

int errorCheckFile(char *inputFileName );


int timer = 2;
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
	int maxChildProcess = 4;
	int numberChildProcess = 2;

	

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

	char errorMessage[100];

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
	

	int errorResult = errorCheckFile(inputFileName);

	if(errorResult == 1) {
		fprintf(stderr,"%s: Error: Inside File have lot of arguments\n",argv[0]);
		return 0;
	}

	//if max child process over 20 error
	if(maxChildProcess > 20){
		fprintf(stderr,"%s: Error: Cannot have 20 or more max process\n",argv[0]);
	}	 


	//signal error
	 if (signal(SIGINT, signalCall) == SIG_ERR) {
        	snprintf(errorMessage, sizeof(errorMessage), "%s: Error: user: signal(): SIGINT\n", argv[0]);
		perror(errorMessage);	
        	exit(errno);
  	  }
	
	//sigalarm error
	if (signal(SIGALRM, signalCall) == SIG_ERR) {
            snprintf(errorMessage, sizeof(errorMessage), "%s: Error: user: signal(): SIGALRM\n", argv[0]);
	     perror(errorMessage);	
         	exit(errno);
     	}
	
	//alarm for 2 real life second
	alarm(timer);
	forkProcess(maxChildProcess, numberChildProcess,inputFileName,outputFileName,increment,argv[0]);
		
		
	return 0;
}	


int errorCheckFile(char *inputFileName ) {

	FILE *f4 = fopen(inputFileName, "r") ;
	int newLineCount = 1;
	int newLineCompare = 0;
	int bufSize = 100;
	char buffer[bufSize];
	int numberLength = 0;
	int fileStackNumberCounter = 0;
	int fileStackNumberCounter2 = 0;
	
	while(fgets(buffer,bufSize,f4)!= 0){
	
		if(newLineCompare == 0){
				int o;
				numberLength = strlen(buffer);	
				for(o=0; o < numberLength; o++){	
					if(isspace(buffer[o]) && !isspace(buffer[o+1])) {										
						fileStackNumberCounter2++;
					}
				}



		}	
		if(newLineCount == newLineCompare) {
			//check if there is 4 numbers terminates
					int o;
					numberLength = strlen(buffer);	
					for(o=0; o < numberLength; o++){	
						if(isspace(buffer[o]) && !isspace(buffer[o+1])) {										
							fileStackNumberCounter++;
						}

					}

		}
		newLineCompare++;
	}	

	if(fileStackNumberCounter > 3 || fileStackNumberCounter2 > 1){
		return 1;
	}

	
	fclose(f4);
	return 0;
}
void forkProcess(int maxChildProcess, int numChildProcess, char *inputFileName, char *outputFileName,int increment,char *arg0Name) {
		
	int arr[2];
	char * args[2];

	int bufSize = 100;
	char buffer[bufSize];
	
	int newLineCount = 1;
	int newLineCompare = 0;

	
	pid_t childpid;
	int ptr_count = 0;
	char errorMessage[1000];

	//check error shmget
	if((shmid = shmget(SHMKEY, sizeof(arr[2]), 0777 | IPC_CREAT )) < 0){
           	
		printf("shmget failed in master\n");	
		exit(1);	
  	}
            	
	//put address    	        
	char * shmaddr = ( char * )( shmat ( shmid, NULL, 0 ) );
        //pointer to the address
	shmPtr = ( int * )( shmaddr);
            	   	                            
	if(shmPtr == -1 ){
            	printf("shmat failed in master");
            	exit(2);	
        }

	shmPtr[0] = 0; 
	shmPtr[1] = 0; 

	
	FILE *f1 = fopen(inputFileName,"r");
				
	if(f1 == NULL){
		 snprintf(errorMessage, sizeof(errorMessage), "%s: Error: ", arg0Name);
	     	perror(errorMessage);	
		return;
	}


	int  line = countLines(f1)-1;

	int stackSize = line *3;
	
	int i,m=0;

	struct Clock clock[stackSize];
 
	//parsing the text
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


	//only run the max child
	while( totalCount < maxChildProcess || ptr_count > 0){
		
		shmPtr[1] += increment;


		//clock incrementation
		if(shmPtr[1] > 1000000000){
			shmPtr[0]++;
			shmPtr[1] -= 1000000000;
		}				
		

			if(waitpid(0, NULL, WNOHANG) > 0){
				ptr_count--;
			}

		//launch time call exec to user.c
		if( ptr_count < numChildProcess && shmPtr[0] == clock[s].seconds && shmPtr[1] > clock[s].nanoseconds){		
			childpid = fork();
			totalCount++;
			ptr_count++;

					
			if(childpid == 0){
				char duration[100];
				sprintf(duration, "%d", clock[s].duration);
				execl("./user","user",duration,outputFileName,(char *)0);
				 snprintf(errorMessage, sizeof(errorMessage), "%s: Error: ", arg0Name);
	     			perror(errorMessage);	
				exit(0);
			
			} else {
				FILE *f3 = fopen(outputFileName,"a+");
				// if file open error and return
				if(f3 == NULL){
					 snprintf(errorMessage, sizeof(errorMessage), "%s: Error: ", arg0Name);
	     				perror(errorMessage);	
				}



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


//signal calls
void signalCall(int signum)
{
    int status;
  //  kill(0, SIGTERM);
    if (signum == SIGINT)
        printf("\nSIGINT received by main\n");
    else
        printf("\nSIGALRM received by main\n");
 
    while(wait(&status) > 0) {
        if (WIFEXITED(status))  /* process exited normally */
                printf("User process exited with value %d\n", WEXITSTATUS(status));
        else if (WIFSIGNALED(status))   /* child exited on a signal */
                printf("User process exited due to signal %d\n", WTERMSIG(status));
        else if (WIFSTOPPED(status))    /* child was stopped */
                printf("User process was stopped by signal %d\n", WIFSTOPPED(status));
    }
    kill(0, SIGTERM);
    //clean up program before exit (via interrupt signal)
    shmdt(shmPtr); //detaches a section of shared memory
    shmctl(shmid, IPC_RMID, NULL);  // deallocate the memory
   
      exit(EXIT_SUCCESS);
 }

 
//count the lines of the file
   
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
		printf("-n int				      | int for max processor\n"); 
		printf("-s int				      | int for max child processor\n"); 
		printf("------------------------------------------------------------------------------------------------------------------------------------------------------\n");
}

