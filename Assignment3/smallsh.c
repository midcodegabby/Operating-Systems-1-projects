/* This is my code for Assignment 3 of OS1. 
 * This code creates a shell with limited functionality:
 * Specifically there are three built in functions: exit, cd, and status.
*/
#define _GNU_SOURCE
#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

//declare a global variable that will allow the parent to go into foreground only mode
int fgMode = 0; //initialize to 0, which means that we are NOT in foreground only mode!
int localMode = 1; //initialize another variable to prevent identical SIGTSTP messages

char* prompt() {

	//this function must output the prompt ": " on every line except when displaying stdout. 
	//must also take in user input.
	
	//print out prompt: (with the process id of the shell running) and then flush stdout
	printf(": ");
	fflush(stdout);
	
	//create a variable to store the command to be inputted by the user
	char* command = NULL;
	
	//buffer size for getline()
	size_t bufferSize = 0; 

	//both the above vars are allocated automatically by getline()
	
	//take user input and then store it in command
	int errorCheck = getline(&command, &bufferSize, stdin);	
	
	if (errorCheck == -1) { //this prevents errors
		clearerr(stdin); //reset stdin
	}
return command;
}

void duringPromptHandler(int signo) {
	//this handles SIGTSTP signals (CTRL-Z) while waiting at the prompt
	
	if (fgMode == 0) { //if SIGTSTP is received and foreground-only mode is off, turn foreground-only mode on
		fgMode = 1;
		char* message1 = "\nEntering foreground-only mode (& is now ignored)\n";
		write(STDOUT_FILENO, message1, 51);

		localMode = 0;
	}
		
	else { //if SIGTSTP is received and foreground-only mode is on, turn foreground-only mode off
		fgMode = 0;
		char* message2 = "\nExiting foreground-only mode\n";
		write(STDOUT_FILENO, message2, 31);

		localMode = 1;
	}
}

void afterPromptHandler(int signo) {
	//this handles SIGTSTP signals (CTRL-Z) after a process is running
	
	if (fgMode == 0) { //if SIGTSTP is received and foreground-only mode is off, turn foreground-only mode on
		fgMode = 1;
	}
		
	else { //if SIGTSTP is received and foreground-only mode is on, turn foreground-only mode off
		fgMode = 0;
	}
}

//main function implements all functionality of the shell
int main() {
	
	int condition = 0;
	
	//initialize an array of ints to store all backgrounded child processes
	int bgChildren[1000] = {0}; //idk how many processes will be run so best to be safe
	int openIdx = 0;
	int maxIdx = 0;

	int exitStatus = 0; //initialize the exitStatus
	int signal = -1; //initialize signal that terminated the child
	int fgChildExitMethod = 0; //initialize the childExitMethod

	//initialize the sigaction structs
	struct sigaction SIGTSTP_action = {0}, ignore_action = {0}, default_action = {0};
	
	SIGTSTP_action.sa_handler = afterPromptHandler; //initial behavior for SIGTSTP should be to turn FG mode on
	sigfillset(&SIGTSTP_action.sa_mask); //block all catchable signals while handle_SIGTSTP is running
	SIGTSTP_action.sa_flags = SA_RESTART; //this is to prevent getline() errors

	ignore_action.sa_handler = SIG_IGN; //ignore signals
	default_action.sa_handler = SIG_DFL; //signal default action

	//use a while loop to stay within the shell for as long as the user has not inputted 'exit' into the shell
	while (condition == 0) { 

		//initialize an int to be the index
		int idx = 0;
		int bgExitMethod = -1;
		int condition2 = 0;
		pid_t bgChildPid = -5;

		//register initial SIGINT and SIGTSTP behavior:
		sigaction(SIGINT, &ignore_action, NULL);
		sigaction(SIGTSTP, &SIGTSTP_action, NULL); 

		//check if this is not the first iteration
		if (bgChildren != 0) {

			while (idx <= maxIdx) { //iterate while there are still children in the array
				if (bgChildren[idx] != 0) { //check if the current index has a childpid
					bgChildPid = waitpid(bgChildren[idx], &bgExitMethod, WNOHANG);
				}

				if (bgChildPid > 0) { //execute if the background child has terminated
					//check for what killed the process
					if(WIFEXITED(bgExitMethod)) { //execute if child process terminated normally
						printf("background pid %d is done: exit value %d\n", bgChildPid, WEXITSTATUS(bgExitMethod));
					}

					else { //execute if child process terminated abnormally (via signal)
						printf("background pid %d is done: terminated by signal %d\n", bgChildPid, WTERMSIG(bgExitMethod));
					}

					//clear that index from the pid array and then set openIndex to it
					bgChildren[idx] = 0;
					openIdx = idx;
					condition2++;
					break;
				}

				//increment idx
				idx++;
			}
			
			//after the while loop is finished, check if anything printed:
			if (condition2 == 0) { //if no terminated children were found
				maxIdx = idx; //set the maximum index to the highest index reached in the while loop
				openIdx = maxIdx;
			} 			
		}

		SIGTSTP_action.sa_handler = duringPromptHandler; //allow messages out during prompt()
		sigaction(SIGTSTP, &SIGTSTP_action, NULL);

		//call the prompt() to setup the prompt of the shell and copy the returned user input to userInput
		char* userInput = prompt();

		SIGTSTP_action.sa_handler = afterPromptHandler; //prevent messages out while execution
		sigaction(SIGTSTP, &SIGTSTP_action, NULL);

		char inputCopy[2050]; //create a duplicate to use strtok_r on
		memset(inputCopy, '\0', 2050);
		strcpy(inputCopy, userInput);

		//use strtok_r to extract the command
		char* saveptr;

		//create an array of char* to store the entire command line in 
		char* commandArray[516];
		commandArray[0] = strtok_r(inputCopy, " \n", &saveptr); 
		int i = 0; 

		//initialize redirect integers all to -1 
		int rOut = -1, rIn = -1; 

		//use a while loop to store all tokens of the command within the commandArray
		while (commandArray[i] != NULL) { //execute while the current token numbered i is not NULL
			i++;
			commandArray[i] = strtok_r(NULL, " \n", &saveptr);

			//use an if statement to return out of the while loop if the current token is NULL
			if (commandArray[i] == NULL) {
				break;
			}
			
			else { //else statement checks for redirect operators
				if (strcmp(commandArray[i], "<") == 0) { //checks if there is stdin redirect
					//update the rIn var to the current index
					rIn = i;
				}
				else if (strcmp(commandArray[i], ">") == 0) { //checks if there is stdout redirect
					//update the rOut var to the current index
					rOut = i;
				}
			}
		}

		//create a variable to store the "&" at the end of a command if it exists
		char amp[2] = "x"; 

		//create a char* to store the expanded command
		char* expandedCommand = malloc(sizeof(char)*strlen(userInput)*12);

		//convert the integer from getpid() to a string using sprintf
		char pid[sizeof(char)*12]; 
		sprintf(pid, "%d", getpid());

		size_t len = 1; //for use in appending a single char to a string
		
		int complicatedInt = 0;

		if (commandArray[0] == NULL) {} //handles blank lines
		
		else if (commandArray[0][0] == '#') {} //handles comment lines

		else {

			//if statement to remove the & token if it exists
			if (strcmp(commandArray[i-1], "&") == 0) {
				strcpy(amp, "&");	
			}
			
			
			//create if statement to check for valididity of a cd command 
			if (commandArray[2] != NULL) {
				
				if (strcmp(commandArray[2], "&") == 0) {
					complicatedInt = 1;
				}	
				
				else {
					complicatedInt = 2;
				}
			}

			//this section should modify the command string by adding in the PID wherever $$ is found
			if (strstr(userInput, "$$")) { //execute if the expansion variable is detected

				//create a char to act as the iterative char and initialize it to the first char of userInput
				char currChar = userInput[0];
				
				char temp[2]; //create a char array to store the char to be apended to the expandedCommand
				memset(temp, '\0', 2);

				int commandLength = strlen(userInput); //length of command
				i = 0; //iteration int
				
				//use a while loop to loop through the entirety of userInput char by char
				while (i < commandLength) {
					
					//if statement handles if i is currently 0
					if (i == 0) {
						
						if (userInput[i] == '$' && userInput[i+1] == '$') { //execute if userInput starts with $$ 
							strcat(expandedCommand, pid);  //insert the pid
							i = i+2; //start after the $$ occurs
						}
						
						else {
							temp[0] = userInput[i];
							strncat(expandedCommand, temp, len); //add the current char to the command
							i = i+1; //move chars by 1
						}
					}	
				
					//else statement handles if i is not 0
					else {
						if (userInput[i] == '$' && userInput[i+1] == '$') { //execute if $$ is found
							strcat(expandedCommand, pid); //insert the pid
							i = i+2; //start after the $$ occurs
						}
						
						else {
							temp[0] = userInput[i];
							strncat(expandedCommand, temp, len); //add the current char to the command
							i = i+1; //move chars by 1
						}
					}	
				}

				//set the commandArray to the expandedCommand
				char* saveptr2;
				commandArray[0] = strtok_r(expandedCommand, " \n", &saveptr2); 
				i = 0; 

				//initialize redirect integers all to -1 
				rOut = -1;
				rIn = -1; 
	
				//use a while loop to store all tokens of the command within the commandArray
				while (commandArray[i] != NULL) { //execute while the current token numbered i is not NULL
					i++;
					commandArray[i] = strtok_r(NULL, " \n", &saveptr2);

					//use an if statement to return out of the while loop if the current token is NULL
					if (commandArray[i] == NULL) {
						break;
					}
			
					else { //else statement checks for redirect operators
						if (strcmp(commandArray[i], "<") == 0) { //checks if there is stdin redirect
							//update the rIn var to the current index
							rIn = i;
					}
						else if (strcmp(commandArray[i], ">") == 0) { //checks if there is stdout redirect
							//update the rOut var to the current index
							rOut = i;
						}
					}
				}
			}
			
  			//execute if userInput and "exit" are the same (accepts & at the end)
			if ((strcmp(userInput, "exit\n") == 0) || (strcmp(userInput, "exit &\n") == 0)) { 
				kill(0, 15); //terminate all processes within the group
			}

			//checks for cd with no path argument
			else if (((strcmp(userInput, "cd\n") == 0) || (strcmp(userInput, "cd &\n") == 0)) &&
				  (commandArray[2] == NULL)) { 
				//change directory to the directory specified by the HOME env var
				char* homeDir = getenv("HOME");
				chdir(homeDir);
			}
			
			//checks for cd with a path argument and will be a super long boolean LOL
			else if (((strcmp(commandArray[0], "cd") == 0) && (commandArray[3] == NULL)) &&  
				  (complicatedInt != 2)) {
				//change the directory to the path given in the command
				chdir(commandArray[1]);
			}	

			//execute if command and "status" are the same (accepts & at end)
			else if ((strcmp(userInput, "status\n") == 0) || (strcmp(userInput, "status &\n") == 0)) { 
				if (signal == -1) { //execute if last foreground process was terminated normally
					printf("exit status %d\n", exitStatus);
				}
				
				else { //execute if last foreground process was terminated abnormally (via signal)
					printf("terminated by signal %d\n", signal);
				}
			}
		
			else { //execute if command is not a function supported by smallsh
				sigaction(SIGTSTP, &ignore_action, NULL); //ensure that the children ignore SIGTSTP

				//create the child and save its pid to a initialized var
				pid_t childPid = -4;
				childPid = fork();
				
				//initialize a buffer to store error messages
				char errorBuffer[2049];
				memset(errorBuffer, '\0', 2049);

				//ensure that SIGINT terminates foreground child processes
				if (strcmp(amp, "&") != 0) {
					sigaction(SIGINT, &default_action, NULL);
				}

				//use if statements to separate actions for the parent and the child process
				if (childPid == -1) { //handles a case where forking fails
					perror("Hull breach");
				}
				
				else if (childPid == 0) { //handles a case where we are currently in the child process
					int check = 0;
		
					sigaction(SIGTSTP, &ignore_action, NULL); //ignore SIGTSTP when in child process			

					//check for stdin and stdout redirection and run dup2() to perform the redirection
					if (rIn != -1) { //check for stdin redirection
						//try to open the file for reading
						int inFile = open(commandArray[rIn+1], O_RDONLY);
						
						//error handling
						if (inFile == -1) { //if we cannot open the file for reading
							//this code only runs if open fails
							sprintf(errorBuffer, "open(): %s", commandArray[rIn+1]);

							perror(errorBuffer); //error message
							exit(1); //exit with error
						}
						
						//redirect stdin from 0 to inFile
						check = dup2(inFile, 0);
							
						if (check == -1) { //error handling
							perror("target dup2()");
							exit(1);
						}
							
						//remove the redirection from the command
						commandArray[rIn] = NULL;
						commandArray[rIn+1] = NULL;
					}

					if (rOut != -1) { //check for stdout redirection
						//try to open the file for writing 
						int outFile = open(commandArray[rOut+1], O_WRONLY | O_CREAT | O_TRUNC, 0640);
						
						//error handling
						if (outFile == -1) {
							//this code only runs if open fails
							sprintf(errorBuffer, "open(): %s", commandArray[rOut+1]);

							perror(errorBuffer); //error message
							exit(1); //exit with error
						}

						//redirect stdout from 1 to outFile
						check = dup2(outFile, 1);
						
						if (check == -1) { //error handling
							perror("target dup2()");
							exit(1);
						}
						
						//remove the redirection from the command
						commandArray[rOut] = NULL;
						commandArray[rOut+1] = NULL;
					}
					
					//do the following redirection when the child is in BG and no input redirection 
					if ((rIn == -1) && (strcmp(amp, "&") == 0) && (fgMode == 0)) {
						//open /dev/null for reading only
						int readingNull = open("/dev/null", O_RDONLY);

						//error handling
						if (readingNull == -1) {
							//this code only runs if open fails
							perror("open(): /dev/null");
							exit(1); //exit with error
						}
						
						//redirect stdin from 0 to /dev/null
						check = dup2(readingNull, 0);

						if (check == -1) { //error handling
							perror("target dup2()");
							exit(1);
						}
					}

					//do the following redirection when the child is in BG and no output redirection 
					if ((rOut == -1) && (strcmp(amp, "&") == 0) && (fgMode == 0)) {
						//open /dev/null for writing only
						int writingNull = open("/dev/null", O_WRONLY);

						//error handling
						if (writingNull == -1) {
							//this code only runs if open fails
							perror("open(): /dev/null");
							exit(1); //exit with error
						}

						//redirect stdout from 1 to /dev/null
						check = dup2(writingNull, 1);	

						if (check == -1) { //error handling
							perror("target dup2()");
							exit(1);
						}
					}

					//if the ampersand is in the commandArray, remove it:
					if (strcmp(amp, "&") == 0) {
						commandArray[i-1] = NULL;
					}

					//call execvp() on the command 
					execvp(commandArray[0], commandArray);
						
					//this code only runs if execvp returns (fails) 
					sprintf(errorBuffer, "execvp(): %s", commandArray[0]);

					perror(errorBuffer); //error message
					fflush(stderr);
					exit(1); //exit with error
				}

				else { //handles a case where we are currently in the parent process
					//change the SIGINT behavior back to ignore
					sigaction(SIGINT, &ignore_action, NULL);

					//change the SIGTSTP behavior back to normal
					sigaction(SIGTSTP, &SIGTSTP_action, NULL);

					//if statement checks for if the child process was in the foreground or background
					if ((strcmp(amp, "&") != 0 ) || (fgMode == 1)){ //execute if child ran in foreground
						//wait for the child to terminate and save the exit method in childExitMethod
						waitpid(childPid, &fgChildExitMethod, 0); 

						//create if statement to print message out if SIGTSTP was received
						if ((fgMode == 1) && (fgMode == localMode)) {
							printf("\nEntering foreground-only mode (& is now ignored)\n");
							fflush(stdout);
							localMode = 0;
						}

						if ((fgMode == 0) && (fgMode == localMode)) {
							printf("\nExiting foreground-only mode\n");
							fflush(stdout);
							localMode = 1;
						}
						
						//update the exit status and signal variables upon child termination
						if (WIFEXITED(fgChildExitMethod)) { //execute if child terminated normally
							exitStatus = WEXITSTATUS(fgChildExitMethod); //update exitStatus
							signal = -1; //reset signal
						}
						
						else { //execute if child terminated abnormally (via signal)
							signal = WTERMSIG(fgChildExitMethod);
							
							//print out a message if the foreground child was terminated via SIGINT
							if ((signal == 2) ) {
								printf("terminated by signal 2\n");
							}
						}
					}
					
					if ((strcmp(amp, "&") == 0) && (fgMode == 0)) { //execute if child ran in background
						//print out a message about the child pid
						printf("background pid is %d\n", childPid);
						
						//add the childPid to the array of child Pids
						bgChildren[openIdx] = childPid;
					}
				}
			}	
		}
		free(expandedCommand);
	}
return 0;
}
