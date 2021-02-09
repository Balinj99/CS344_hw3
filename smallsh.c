#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>


void printExitValOrSignal(int exitStatus){
    	if(WIFEXITED(exitStatus)){
           	 printf("exit value %d\n", WEXITSTATUS(exitStatus));
    	} 
	else if (WIFSIGNALED(exitStatus)){
		printf("terminated by signal %d\n", WTERMSIG(exitStatus));        
    	}
}

void checkBG(){
	int pid, status;

	while((pid = waitpid(-1, &status, WNOHANG)) > 0){
	   	
		printf("background pid %d is done: ", pid);
		printExitValOrSignal(status);
		fflush(stdout);
	}

}


int main(){
   	//spawnpid for forks
   	pid_t spawnPid = -5;
	int childStatus, fd, i, j;
	//chars for use in extracting command and changing directory for cd
	char dir[512] = {0};
	char* token;
	char* token2;
	char* saveptr;
        char* saveptr2;
	//flag to check if any non built-in commands have been run
	int extStat = 0;
	char* newargv[512] = {0};
	int bfFlag, ignoreFlag = 0;
	
	//printf("smallsh pid is: %d\n", getpid());

	while(1){
	   	//char for input and prompt for the user
		memset(newargv, 0, sizeof(newargv));
	   	char input[512] = "";
		int spaceCount = 0;
		checkBG();
		printf(": ");
		fflush(stdout);
		//changed scanf format to accept input string with spaces
		scanf("%[^\n]%*c", input);
		//fflush(stdin);
		
		for(i = 0; i < strlen(input); i++){
			if(input[i] == ' '){
				spaceCount++;
			}
		}
		//printf("space cnt: %d\n", spaceCnt);
		//case where user input is a comment		
		if(strncmp(input, "#", 1) == 0){
			//extStat = 0;
		   	continue;
		}
		
		//case where the input is blank
		else if(input[0] == '\0'){
		   	fgetc(stdin);
		   	//extStat = 0;
			continue;
		}
	
		//buit-in commands	
		//case where user enters exit
		else if(strcmp(input, "exit") == 0){
		   	//add at exit statement here to clean up processes
			exit(0);
		}
		
		//user enters a cd command
		else if(strncmp(input, "cd", 2) == 0){
		   	//check if the user enters a blank cd change to the HOME directory
			if(strlen(input) <= 3){
				chdir(getenv("HOME"));
			}
			
			//if the cd command has an argument, extract the path from the argument and change to that directory
			else{
			   	//extract the command using a token 
				token = strtok_r(input, " ", &saveptr);
				token = strtok_r(NULL, " ", &saveptr);
				chdir(token);
			}
			extStat = 0;
			//printf("%s\n", getcwd(dir, 512));
			//fflush(stdout);
		}
		
		//user enters status command
		else if(strncmp(input, "status", 6) == 0){
			printf("exit value %d\n", extStat);
			fflush(stdout);
			extStat = 0;
		}

		//if any non built-in command is entered
		else{
		   	//for loop to extract new process from command
			for(i = 0; i < spaceCount + 1; i++){
			   	//check if the input is just starting to be parsed
				if(i == 0){
					//start making the command for the new process using the user input
		   			newargv[i] = strtok_r(input, " ", &saveptr);
				}
				//after the first part of the command is saved into newargv
				else{
				   	//extract the next part of the command and save it to newargv
					token = strtok_r(NULL, " ", &saveptr);
						
					//check if the final token in the command is &
					//this indicates the process should run in the background
					if((strcmp(token, "&") == 0) && i == spaceCount){
						//printf("now in back ground\n");
						//fflush(stdout);
						//this flag indicates that the process will be in the background
						bfFlag = 1;
						break;
					}
					//otherwise add the token to the next position in newargv
					else{								
						//chek if a token is $$
						//if it is, replace it with the current PID (that of smallsh)
						
										
						if(strcmp(token, "$$") == 0){
					   		sprintf(token, "%d", getpid());
							fflush(stdout);
						}
						/*
						else{
							for(j = 0; j < strlen(token); j++){
								if(((i+1) < strlen(token)) && (token[i] == '$') && (token[i+1] == '$')){
									token2 = strtok_r(token, "$", &saveptr2);
									//token3 = strtok_r(NULL, " ", &saveptr2);
									sprintf(token, "%s%d", token2, getpid());
								}
							}

						}
						*/
						newargv[i] = token;
					}
				}
			}
			
			//set final argument to NULL 
			//this indicating to the process that there are no more arguments
			newargv[i+1] = NULL;
		
			//fork to start running a new child process
			spawnPid = fork();
			switch(spawnPid){

			   	//if there is an error forking
				case -1:
					perror("fork() failed!");
					fflush(stdout);
					exit(1);
					break;

				//if the fork succeedes
				case 0:
					//exec to replace this process with a different one
					/*
					//if one of the arguments is a < or a >, open the file/ files given in newargv as the command
					//then use dup2 to redirect stdin and/or stdout into the given files
					if(strcmp(newargv[1], ">") == 0){
						fd = open(newargv[2], O_RDWR | O_CREAT | O_TRUNC, 0640);
						int result = dup2(fd, 1);
					}
                                        */	
					//use execvp to exchange the child process (a copy of smallsh) with the process specified by newargv				
					execvp(newargv[0], newargv);
					
					//in the case that execvp return, there is an error
					perror("execvp() failed!");
					fflush(stdout);
					exit(2);
					break;

				//after the child is executed
				default:
						
					//check the flag to see if the process is to be run in the background or foreground
					//in this case the process is to be run in the foreground
					if(bfFlag == 0){
						spawnPid = waitpid(spawnPid, &childStatus, 0);
					}
					//otherwise, the program is to be run in the background
					else if(bfFlag == 1){
						printf("background pid is %d\n", spawnPid);
						fflush(stdout);
						//int chPid = spawnPid;
						//spawnPid = waitpid(spawnPid, &childStatus, WNOHANG);
						//printf("background pid %d is done: \n", chPid);
						bfFlag = 0;
					}
					
					//check that the child process exited
					//then change the extStat variable to be the exit value so it can be viewd by the stat command
					if(WIFEXITED(childStatus)){
						extStat = WEXITSTATUS(childStatus);
					}

					break;
			}
		}
   
		/*
                //user inputs an incorrect command
		else{
			printf("Command not recognized.\n");
			continue;
		}
		*/		
	}
	
	return 0;
}
