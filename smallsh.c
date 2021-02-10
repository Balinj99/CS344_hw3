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


//function that checks for the status of a process and prints out the appropriate message depending on if its an exit value or a termination signal
//I got help with this function in office hours
void printExitValOrSignal(int exitStatus){
   	//check if the process was exited and print the exit status
    	if(WIFEXITED(exitStatus)){
           	 printf("exit value %d\n", WEXITSTATUS(exitStatus));
    	} 

	//check if the process was terminated and print the signal
	else if(WIFSIGNALED(exitStatus)){
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
	int childStatus, fd, fd2, i, j;
	//chars for use in extracting command and changing directory for cd
	char dir[512] = {0};
	char* token;
	//char* token2;
	char* saveptr;
        //char* saveptr2;
	//flag to check if any non built-in commands have been run
	int extStat, extSig = 0;
	char* newargv[512] = {0};
	int bfFlag, ignoreFlag, sigFlag = 0;
	//printf("smallsh pid is: %d\n", getpid());
	char input[512] = "";

	while(1){
	   	//char for input and prompt for the user
		memset(newargv, 0, sizeof(newargv));
		char* infile;
		char* outfile;		
		//char input[512] = "";
		//char* input;
		//size_t inSize = 512;
		//size_t inCnt;
		int spaceCount = 0;
		int inFlag, outFlag;
		checkBG();
		printf(": ");
		fflush(stdout);
		//changed scanf format to accept input string with spaces
		scanf("%[^\n]%*c", input);
		//inSize = getline(&input, &inSize, stdin);
		inFlag = 0;
		outFlag = 0;

		for(i = 0; i < strlen(input); i++){
			if(input[i] == ' '){
				spaceCount++;
			}
		}
		
		//printf("space cnt: %s\n", input);
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
		   	/*
		   	//check if the last process exited
		   	if(sigFlag == 0){
				printf("exit value %d\n", extStat);
			}

			//otherwise check if the last process was terminated
			else if(sigFlag == 1){
				printf("terminated by signal %d\n", extSig);
			}
			*/
			fflush(stdout);
			//set exit status and signal flag to 0 incase the user inputs another built-in command
			extStat = 0;
			//sigFlag = 0;
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

					//otherwise check for i/o redirection if output is to be redirected
					else if(strcmp(token, ">") == 0){
						outFlag = 1;
						outfile = strtok_r(NULL, " ", &saveptr);
						break;
					}
					
					//more checking for i/o redirection if input is to redirected
					else if(strcmp(token, "<") == 0){
						inFlag = 1;
						infile = strtok_r(NULL, " ", &saveptr);
						//printf("infile found %s\n", infile);
						//fflush(stdout);
						if((i + 2) < (spaceCount + 1)){
							token = strtok_r(NULL, " ", &saveptr);
							if(strcmp(token, ">") == 0){
								outFlag = 1;
								outfile = strtok_r(NULL, " ", &saveptr);
								//printf("outfile found %s\n", outfile);
								//fflush(stdout);
							}
						}

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
			//printf("before switch infile is: %s\n", infile);
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
					
					//if one of the arguments is a < or a >, open the file/ files given in newargv as the command
					//then use dup2 to redirect stdin and/or stdout into the given files
					
					if(outFlag == 1){
						fd = open(outfile, O_RDWR | O_CREAT | O_TRUNC, 0640);
						int result = dup2(fd, 1);
					}

					if(inFlag == 1){
						fd2 = open(infile, O_RDONLY);
						//printf("infile is: %s\n", infile);
						//fflush(stdout);	
						
						if(fd2 < 0){
							printf("cannot open %s for input\n", infile);
							fflush(stdout);
							//fflush(stdin);
							exit(1);
						}

						else{
							int result2 = dup2(fd2, 0);
						}
						
						//int result2 = dup2(fd2, 0);
					}
                                        	
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
					
					//check if the child process exited
					//then change the extStat variable to be the exit value so it can be viewd by the stat command
					if(WIFEXITED(childStatus)){
					   	//set extStat to the exit status so it can be seen using status
						extStat = WEXITSTATUS(childStatus);
						//set sigFlag to 0 indicating to status that the last process was exited
						sigFlag = 0;
					}
					/*
					//otherwise, check if the child process was terminated
					else if(WIFSIGNALED(childStatus)){
					   	//set the extSig to the termination signal so it can be viewed via status
						extSig = WTERMSIG(childStatus);
						//set sigFlag to 1 indicating to status that the process was terminated
						sigFlag = 1;
					}
					*/
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
