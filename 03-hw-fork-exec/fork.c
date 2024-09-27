#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

int main(int argc, char *argv[]) {
	int pid;
	//int *childStatus = 0;
	FILE* filePointer = NULL;
	int fd[2];


	printf("Starting program; process has pid %d\n", getpid());
 	
	filePointer = fopen("fork-output.txt","w");
	//fprintf(filePointer, "%d", fileno(filePointer));
	
	fprintf(filePointer, "BEFORE FORK (%d)\n", fileno(filePointer));
	fflush(filePointer);


	int lame = pipe(fd);
	lame++;

	if ((pid = fork()) < 0) {
		fprintf(stderr, "Could not fork()");
		exit(1);
	}

	/* BEGIN SECTION A */

	printf("Section A;  pid %d\n", getpid());
	//sleep(5);

	/* END SECTION A */
	if (pid == 0) {
		/* BEGIN SECTION B */
		dup2(fileno(filePointer), 1);
		printf("Section B\n");
		//sleep(5);
		close(fd[0]);
//		sleep(10);
		int writething = write(fd[1], "hello from Section B\n", 21);
		writething++;
//		sleep(10);
		close(fd[1]);

		
		fprintf(filePointer, "SECTION B (%d)\n", fileno(filePointer));
		fflush(filePointer);		
		//sleep(30);
		//printf("Section B done sleeping\n");



		char *newenviron[] = { NULL };
		printf("Program \"%s\" has pid %d. Sleeping.\n", argv[0], getpid());
//		sleep(8);
		if (argc <= 1) {
			printf("No program to exec.  Exiting...\n");						            
			exit(0);									       
		}

						        
		printf("Running exec of \"%s\"\n", argv[1]);
							        
		execve(argv[1], &argv[1], newenviron);
								        
		printf("End of program \"%s\".\n", argv[0]);
		exit(0);

		/* END SECTION B */
	} else {
		/* BEGIN SECTION C */

		printf("Section C\n");
		
		char myBuf[30];
		close(fd[1]);
		int numRead = read(fd[0], myBuf, 21);
		printf("%d \n", numRead);
		numRead = read(fd[0], myBuf, 21);
		myBuf[numRead] = 00;
		printf("%d \n", numRead);
		printf("%s s", myBuf);
		close(fd[0]);	


		fprintf(filePointer, "SECTION C (%d)\n", fileno(filePointer));
		fclose(filePointer);
		close(fileno(filePointer));
		//sleep(5);
		
	
		//wait(childStatus);
		//sleep(30);
		//printf("Section C done sleeping\n");

		exit(0);

		/* END SECTION C */
	}
	/* BEGIN SECTION D */

	printf("Section D\n");
	//sleep(30);

	/* END SECTION D */
}

