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

int main(){
   	//spawnpid for forks
   	pid_t spawnPid = -5;
	int childStatus;
	//chars for use in extracting command and changing directory for cd
	char dir[512] = {0};
	char* token;
	char* saveptr;
	//flag to check if any non built-in commands have been run
	int extStat = 0;
	char* newargv[] = {"ls", NULL};

	while(1){
	   	//char for input and prompt for the user
	   	char input[512] = "";
		printf(": ");
		fflush(stdout);
		//changed scanf format to accept input string with spaces
		scanf("%[^\n]%*c", input);
		fflush(stdin);
		
		//case where user input is a comment		
		if(strncmp(input, "#", 1) == 0){
			continue;
		}
		
		//case where the input is blank
		else if(input[0] == '\0'){
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

			printf("%s\n", getcwd(dir, 512));
			fflush(stdout);
		}
		
		//user enters status command
		else if(strncmp(input, "status", 6) == 0){
			printf("exit value %d\n", extStat);
			fflush(stdout);
		}

		//if any non built-in command is entered
		else{
		   	int i = 0;
			//extract new process from command
			newargv[i] = strtok_r(input, " ", &saveptr);
			i++;

			token = strtok_r(NULL, " ", &saveptr);
			//printf("Token: %s\n", token);
			newargv[i] = token;
			i++;

			/*
			if(!(strcmp(token, "<") == 0) && !(strcmp(token, ">") == 0) && (token[0] != '\0')){
				newargv[i] = token;
				i++;
			}
			*/

			newargv[i] = NULL;
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
					execvp(newargv[0], newargv);
					
					//in the case that execvp return, there is an error
					perror("execvp() failed!");
					fflush(stdout);
					exit(2);
					break;

				//after the child is executed
				default:
					//wait for the child process to complete
					spawnPid = waitpid(spawnPid, &childStatus, 0);
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
