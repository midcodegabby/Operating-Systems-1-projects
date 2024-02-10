//This program is the encryption server and will run in the background as a daemon.
//written by Gabriel Rodgers in Operating Systems 1.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Error function used for reporting issues
void error(const char *msg) {
	perror(msg);
	exit(1);
} 

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, int portNumber){

	// Clear out the address struct
	memset((char*) address, '\0', sizeof(*address)); 

	// The address should be network capable
	address->sin_family = AF_INET;

	// Store the port number
	address->sin_port = htons(portNumber);

	// Allow a client at any address to connect to this server
	address->sin_addr.s_addr = INADDR_ANY;
}

int chartonum(char x) {
	//this function will convert the given char to an int
	
	if (x == ' ') { return 26; }
	
	else {
		return ((int) x - 65); //this converts the given char to ascii value and subtracts by 65 to get 0 for 'A'
	}
}
		

int encrypt(char* plaintext, char* key, char* ciphertext) {
	//This function will encrypt the plaintext using the given key.
	
	int len = strlen(plaintext);
	int i = 0;
	int val;
	
	//create a for loop to encrypt the plaintext
	//for (i = 0; i < len; i++) {
	while (i < len) {
		val = chartonum(plaintext[i]) + chartonum(key[i]);
		val %= 27;

		//check if the remaining number is a space 
		if (val == 26) { 
			ciphertext[i] = ' ';
		}
		
		else { //remaining number is not a space
			ciphertext[i] = (char) (val + 65);
		}
	
		i++;

	}

return 0;
}

int decrypt(char* ciphertext, char* key, char* plaintext) {
	//This function will decrypt the plaintext using the given key.
	
	int len = strlen(ciphertext);
	int i = 0;
	int val;
	
	//create a for loop to encrypt the plaintext
	//for (i = 0; i < len; i++) {
	while (i < len) {
		val = chartonum(ciphertext[i]) - (chartonum(key[i]) % 27);

		if (val < 0) {
			val += 27;
		}
		
		//check if the remaining number is a space (if i is equal to zero then it is a space)
		if (val == 26) { 
			plaintext[i] = ' ';
		}
		
		else { //remaining number is not a space
			plaintext[i] = (char) (val + 65);
		}

		i++;
	}

return 0;
}

int recvsz(int sock) {
	//This function will receive an int in the form of a char array from the socket
		
		char buffer[20];
		int totChars = 0;
		int charsRead = 0;
		memset(buffer, '\0', sizeof(buffer));

		while (totChars < sizeof(buffer)) {
			charsRead = recv(sock, buffer, sizeof(buffer), 0);
	
			if (charsRead < 0) { error("SERVER: ERROR reading from socket"); }

			totChars += charsRead;
		}
								
return atoi(buffer); //convert the buffer to int and return it
}

int recvstr(int sock, char* str, int size) {
	//This function will receive a string from a socket.
	
		int totChars = 0;
		int charsRead = 0;
		//char buffer[200];
		//memset(buffer, '\0', sizeof(buffer));

		char* buffer = malloc(200*sizeof(char));
		memset(buffer, '\0', 200*sizeof(char));

		while (totChars < size + 1) {

			//printf("SERVER: totChars = %d\n", totChars);
			charsRead = recv(sock, buffer, 200, 0);

			//printf("SERVER: charsRead = %d\n", charsRead); fflush(stdout);
			if (charsRead < 0) { error("SERVER: ERROR reading from socket"); }

			totChars += charsRead;
													
			//add the buffer to the str
			if (totChars == 0) {
				strncpy(str, buffer, charsRead);
			}
				
			else {
				strncat(str, buffer, charsRead);
			}
			//strcat(str, buffer);
			
			//printf("SERVER: after the strncat\n"); fflush(stdout);
			
			charsRead = 0;

			memset(buffer, '\0', 200*sizeof(char));
		}
	free(buffer);
	return 0;
}

int sendstr(int sock, char* str) {
	//This function will send a string to a socket.
	
	int totChars = 0;
	int charsWritten = 0;
	int size = strlen(str);
	//char* buffer[200];
	//memset(buffer, '\0', sizeof(buffer));

	char* buffer = malloc(200*sizeof(char));
	memset(buffer, '\0', 200*sizeof(char));

	//while loop will send data until all chars are sent
	while (totChars < size + 1) {
		
		strncpy(buffer, str + totChars, 200); //copy 200 bytes of str to buffer
		charsWritten = send(sock, buffer, 200, 0); //send 200 bytes (includes the null term)

		if (charsWritten < 0) { error("SERVER: ERROR writing to socket"); }

		totChars += charsWritten; //add charsWritten to totChars
		charsWritten = 0;
		memset(buffer, '\0', 200*sizeof(char)); //reset the buffer
	}
	free(buffer);
	return 0;
}

int checkConnection(int sock) {
	//this function checks if a given connection is with the correct party
	
	char buffer[11];
	memset(buffer, '\0', 11);

	//send a string to the client
	if (send(sock, "DEC_SERVER", 11, 0) < 0) { error("SERVER: ERROR writing to socket"); }

	if (recv(sock, buffer, 11, 0) < 0) { error("SERVER: ERROR reading from socket"); }
	
	if (strcmp(buffer, "DEC_CLIENT") == 0) { //if the connection is with enc_client
		return 0;
	}

	else {
		return 1;
	}
}

int main(int argc, char *argv[]){

	int connectionSocket;
	//char buffer[200];
	//memset(buffer, '\0', sizeof(buffer));
	struct sockaddr_in serverAddress, clientAddress;
	socklen_t sizeOfClientInfo = sizeof(clientAddress);

	// Check usage & args
	if (argc < 2) { 
		fprintf(stderr,"USAGE: %s port\n", argv[0]); 
		exit(1);
	} 

	// Create the socket that will listen for connections
	int listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (listenSocket < 0) {
		error("ERROR opening socket");
	}

	// Set up the address struct for the server socket
	setupAddressStruct(&serverAddress, atoi(argv[1]));

	// Associate the socket to the port
	if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
		error("ERROR on binding");
	}

	// Start listening for connetions. Allow up to 5 connections to queue up
	listen(listenSocket, 5);
 	//char* plaintext = NULL;
	//char* key = NULL;

	int children[5] = {0}; //initialize an int array to all zeros
	int exitMethod = 0;
	int openI = 0;
	int i;

	// Accept a connection, blocking if one is not available until one connects
	while(1){
		
		// Accept the connection request which creates a connection socket
		connectionSocket = accept(listenSocket,	(struct sockaddr *)&clientAddress, &sizeOfClientInfo); 

		if (connectionSocket < 0){
			error("SERVER: ERROR on accept");
		}
		
		//check if we just connected to ENC_CLIENT or not
		if (checkConnection(connectionSocket) == 1) {
			perror("SERVER: Port number not owned by DEC_CLIENT");
		}

		else { 
			
			//initialize forking stuff
			pid_t bgChildPid = -5;

			if (children != 0) { //check for children that terminated
				for (i = 0; i < 5; i++) {
					if (children[i] != 0) { //wait for the child if it exists
						bgChildPid = waitpid(children[i], &exitMethod, WNOHANG); 	
					}

					if (bgChildPid > 0) { //execute if the child exited
						children[i] = 0;
						openI = i;
					}
				}
			}

			i = 0;

			//create the child and save its pid to an initialized var
			pid_t childPid = -4;
			childPid = fork();

			if (childPid == -1) { //bad
				perror("SERVER: ERROR in forking");
			}
			
			else if (childPid == 0) { //we are in the child process
				int cipherSize  = recvsz(connectionSocket);

				//printf("SERVER: plainSize = %d\n", plainSize);

				//initialize the char* to hold the plaintext
				
				char* ciphertext = malloc(cipherSize*sizeof(char)); 

				memset(ciphertext, '\0', cipherSize*sizeof(char));

				recvstr(connectionSocket, ciphertext, cipherSize);

				//int pSize = plainSize;

				//printf("SERVER: plaintext = %s\n", plaintext);

				int keySize = recvsz(connectionSocket);
				
				//printf("SERVER: keySize = %d\n", keySize);
			
				char* key = malloc(keySize*sizeof(char)); 

				memset(key, '\0', keySize*sizeof(char));

				recvstr(connectionSocket, key, keySize);

				//printf("SERVER: key = %s\n", key);

				char* plaintext = malloc(cipherSize*sizeof(char));

				memset(plaintext, '\0', cipherSize*sizeof(char));

				decrypt(ciphertext, key, plaintext);

				//printf("SERVER: ciphertext = %s\n", ciphertext);

				sendstr(connectionSocket, plaintext);

				// Close the connection socket for this client
				free(plaintext);
				free(key);
				free(ciphertext);
				close(connectionSocket); 

				exit(0);
			}

			else { //we are in the parent process 
				//add the childPid to the children array
				children[openI] = childPid;
			}
		}
	}
	
	// Close the listening socket
	close(listenSocket); 

	return 0;
}
