

#define _GNU_SOURCE
#define BILLION 1E9
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h> 

int my_system(char *command);	//prototype of the function used for the command executions

char *mode;			//Global variables for the pipe version
char *pipePath;
int pipeStatus;			//pipeStatus = 0 means piping instructions were not correct, 1 means piping is a go

int main(int argc, char *argv[]){
	//Array that will store the user command
	char buffer[2000];
	//first argument will be path of pipe, second will either be r for read or w for write
	
	/********Testing variables******/
	//double diff;
	//struct timespec end, start;
	/*******************************/


	//Check for correct piping instructions
	#ifdef PIPE	
	if(argc == 3){
		
		pipePath = argv[1];
		mode = argv[2];
		int error = mkfifo(pipePath,0666);

		if(error == -1){	//check what the error is
			if(errno != EEXIST){		//if the error is not due to the pipe already existing
				printf("Error creating the pipe, please make sure correct arguments were given. Shell piping not set");
				pipeStatus=0;
			}
			//also make sure the read or write commands were given correctly
			else if(strcmp(mode,"r")==0 || strcmp(mode,"w") ==0 || strcmp(mode,"W")==0 || strcmp(mode,"R")==0){
				 pipeStatus = 1;
			}
			else{
				pipeStatus = 0;
				printf("Error creating the pipe, please make sure correct arguments were given. Shell piping not set");
			}
		}
		else if(strcmp(mode,"r")==0 || strcmp(mode,"w")==0 || strcmp(mode,"W")==0 || strcmp(mode,"R")==0){
			pipeStatus = 1;
		}
		else {
			pipeStatus = 0;
			printf("Error creating the pipe, please make sure correct arguments were given. Shell piping not set");
		}
	}
	#endif


	printf("\ntshell>");
	
	/**********Start clock***************/
	//clock_gettime(CLOCK_REALTIME,&start);
	/************************************/

	while(fgets(buffer, sizeof(buffer), stdin)!=NULL){	//loop forever
		printf("tshell>");
		char *command = strtok(buffer,"\n");	//so we get rid of carriage return

		if(strlen(command) <2) exit(0);
		if(strcmp(command,"exit")!=0&&strlen(command)>1){	//if user does not want to exit and valid input
			
			if(my_system(command) != 0) fprintf(stderr,"\nerror in command\n");
		}
		else exit(0);
		
	}

	/***********Stop clock***************/
	//clock_gettime(CLOCK_REALTIME,&end);
	//diff = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/BILLION;
	//printf("\nsystem() execution time: %lf",diff);
	/************************************/
	

}

//Function to be executed in clone version
int execCmd(void *arg){
	char *command = (char *) arg;
	execl("/bin/sh", "sh", "-c", command, (char *) 0);
	perror("execl");	//this only occurs if there was an error during execl
	exit(1);
}

int my_system(char *command){

	/**********fork() version******************/
	#ifdef FORK

	int pid = fork();	//initiate fork
	if(pid == 0){
		execl("/bin/sh", "sh", "-c", command, (char *) 0); //child
		perror("execl");	//execl only returns if error
		_exit(1);
	}
	else if(pid < 0) {
		fprintf(stderr,"Command + %s + could not be executed.", command);
		return -1;	//exit with error
	}
	else {
		wait(&pid);
		return 0;	//exit success
	}
	/*******************************************/


	/**********vfork() version******************/
	#elif VFORK

	int pid = vfork();	//initiate fork
	if(pid == 0){ 
		execl("/bin/sh", "sh", "-c", command, (char *) 0); //child
		perror("execl");	//execl only returns if error
		_exit(1);
	}
	else if(pid == -1) {
		printf("Command + %s + could not be executed.", command);
		return -1;	//exit with error
	}
	else {
		wait(&pid);	
		return 0;	//exit success
	}
	/*******************************************/


	/**********clone() version******************/
	#elif CLONE
	
	//create stack
	void * stack;
	int stackSize = 1024 * 1024;
	stack = malloc(stackSize);

	if(stack == NULL) return -1;

	int pid = clone(execCmd, stack + stackSize, CLONE_FS | CLONE_VFORK, command);	//initiate clone
	if(pid == -1) {
		fprintf(stderr,"Command + %s + could not be executed.", command);
		return -1;	//exit with error
	}
	else {
		wait(&pid);
		free(stack);	
		return 0;	//exit success
	
	}
	/*******************************************/

	/*****************pipe version******************/
	#elif PIPE
		int pid = fork();	//initiate fork

	if(pipeStatus == 1){
		if(pid == 0){
			if(strcmp(mode,"w")==0 || strcmp(mode,"W") ==0){
				//open write end of pipe
				close(1);	//close stdout
				int fd;
				while(open(pipePath, O_WRONLY)<0);	//keep trying to open pipe until successful
				//once opened, write end of path hooked to stdout!

				execl("/bin/sh", "sh", "-c", command, (char *) 0); //child
				perror("execl");	//execl only returns if error
				_exit(1);
			}
			if(strcmp(mode,"r")==0 || strcmp(mode,"R") ==0){
				close(0);
				int fd;
				while(open(pipePath, O_RDONLY)<0);	//keep trying to open read pipe until successful
			
				execl("/bin/sh", "sh", "-c", command, (char *) 0);
				perror("execl");
				_exit(1);
			}
		
		}
		else if(pid < 0) {
			printf("Command + %s + could not be executed.", command);
			return -1;	//exit with error
		}
		else {
			wait(&pid);
			return 0;	//exit success
		}
	}
	
	//If there was an error getting the pipe then we run the regular shell
	else {
					
		if(pid == 0){
			execl("/bin/sh", "sh", "-c", command, (char *) 0); //child
			perror("execl");	//execl only returns if error
			_exit(1);
		}
		else if(pid < 0) {
			printf("Command + %s + could not be executed.", command);
			return -1;	//exit with error
		}
		else {
			wait(&pid);
			return 0;	//exit success
		}


	}


	/***********************************************/


	/**********regular system call******************/
	#else
		if(system(command) == -1) return -1;
	#endif
	/***********************************************/
}


