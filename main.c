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
void forkProcess(int maxChildProcess, int numberChildProcess);
	
int timer = 2;
int shmid;
int *shmPtr;

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

	printf("%s",buffer);
	forkProcess(maxChildProcess, numberChildProcess);
		
		
	return 0;
}	


void forkProcess(int maxChildProcess, int numChildProcess) {
		
	int i, status, arr[2];

	pid_t childpid;

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

	shmPtr[0] = 1000; 
	shmPtr[1] = 1000; 
	
	for (i = 0;i < 1; i++){
		
		childpid = fork();

		int status;
		//fork Starts
		if(childpid < 0) {
			//snprintf(errorMessage, sizeof(errorMessage), "%s: Error", arg0Name);
			//perror(errorMessage);	
		} else if (childpid == 0){	
	        	execl("./user", "user", NULL);
        	
		} else {
			wait(&status);	
		}
	}
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

