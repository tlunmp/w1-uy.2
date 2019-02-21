#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>	

void helpMenu();

int main (int argc, char *argv[]) {

	char inputFileName[] = "input.dat";
	char outputFileName[] = "output.dat";
	int bufSize = 100;
	char buffer[bufSize];
		
	int newLineCount = 0;
	int c;
	

	//getopt command for command line
	while((c = getopt (argc,argv, "hi:o:")) != -1) {

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
			case 'n':
				break;
			case 's':
				break;
			default:
				fprintf(stderr, "%s: Error: need argument -i or -o\n", argv[0]);
				break;	
		}


	}
	return 0;
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

