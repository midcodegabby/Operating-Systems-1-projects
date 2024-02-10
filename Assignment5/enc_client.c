//This program connects to enc_server and asks enc_server to perform a one-time pad style encryption.
//Written by Gabriel Rodgers in Operating Systems 1.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/**
 * * Client code
 * * 1. Create a socket and connect to the server specified in the command arugments.
 * * 2. Prompt the user for input and send that input as a message to the server.
 * * 3. Print the message received from the server and exit the program.
 * */

//Error function used for reporting issues
void error(const char *msg) { 
   	perror(msg); 
     	exit(1); 
}

//set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber, 
                        char* hostname){
 
	//clear out the address struct
	memset((char*) address, '\0', sizeof(*address));

  	//The address should be network capable
	address->sin_family = AF_INET;
	      
	//store the port number
	address->sin_port = htons(portNumber);
	
        // Get the DNS entry for this host name
        struct hostent* hostInfo = gethostbyname(hostname); 
        if (hostInfo == NULL) { 
        	fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
        	exit(0); 
        }

        // Copy the first IP address from the DNS entry to sin_addr.s_addr
        memcpy((char*) &address->sin_addr.s_addr, 
        	hostInfo->h_addr_list[0],
        	hostInfo->h_length);
}

int chartonum(char x) {
	//this function will convert the given char to an int
	
	if (x == ' ') { return 26; }
	
	else {
		return ((int) x - 65); //this converts the given char to ascii value and subtracts by 65 to get 0 for 'A'
	}
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

int sendstr(int sock, char* str) {
	//This function will send a string to a socket.
	
	int totChars = 0;
	int charsWritten = 0;
	int size = strlen(str);
	//char buffer[200];
	//memset(buffer, '\0', sizeof(buffer));
	
	char* buffer = malloc(200*sizeof(char));
	memset(buffer, '\0', 200*sizeof(char)); //reset the buffer

	//printf("CLIENT: buffer = %s\n", buffer);
	//strncpy(buffer, str + totChars, 200);
	//printf("CLIENT: buffer = %s\n", buffer);

	//while loop will send data until all chars are sent
	while (totChars < size + 1) {
		
		strncpy(buffer, str + totChars, 200); //copy 200 bytes of str to buffer
		charsWritten = send(sock, buffer, 200, 0); //send 200 bytes (includes the null term)

		if (charsWritten < 0) { error("CLIENT: ERROR writing to socket"); }

		totChars += charsWritten; //add charsWritten to totChars
		//printf("CLIENT: buffer = %s\n", buffer);
		memset(buffer, '\0', 200*sizeof(char)); //reset the buffer

	}
	free(buffer);
	return 0;
}

int sendsz(int sock, int sz) {
	//This function will send an int in the form of a char array to the socket
	
	char buffer[20]; 
	int totChars = 0;
	int charsWritten = 0;
	memset(buffer, '\0', sizeof(buffer));
	
	sprintf(buffer, "%d", sz); //convert sz to string and load it into the buffer

	while (totChars < sizeof(buffer)) {
		charsWritten = send(sock, buffer + totChars, 20, 0); 

		if (charsWritten < 0) { error("CLIENT: ERROR writing to socket"); }

		totChars += charsWritten; 
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
		charsRead = recv(sock, buffer, 20, 0);

		if (charsRead < 0) { error("CLIENT: ERROR reading from socket"); }

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

	while (totChars < size + 1) {
		charsRead = recv(sock, buffer, 200, 0);

		if (charsRead < 0) { error("CLIENT: ERROR reading from socket"); }

		totChars += charsRead;
		
		//add the buffer to the str
		if (totChars == 0) {
			strncpy(str, buffer, charsRead);
		}

		else {
			strncat(str, buffer, charsRead);
		}

		charsRead = 0;

		memset(buffer, '\0', 200*sizeof(char));
	}
	
	free(buffer);
	return 0;

}	

int checkConnection(int sock) {
	//This function will check if a connection is with the correct server.
	
	char buffer[11];
	memset(buffer, '\0', 11);

	if (recv(sock, buffer, 11, 0) < 0) { error("CLIENT: ERROR reading from socket"); }

	if (send(sock, "ENC_CLIENT", 11, 0) < 0) { error("CLIENT: ERROR reading from socket"); }

	if (strcmp(buffer, "ENC_SERVER") == 0) { //if the buffer and "1" are the same
		return 0;
	}

	else { 
		return 1;
	}
}

int main(int argc, char *argv[]) {
	
	//initalize a bunch of stuff
	int socketFD; 
	//int portNumber;
	struct sockaddr_in serverAddress;

	//Check usage & args
	if (argc < 4) { 
		error("More arguments required");
        } 
	
	//open the key file and the plaintext file
	FILE *keyFile = fopen(argv[2], "r"); //open for read only
	FILE *plainFile = fopen(argv[1], "r"); //open for read only

	//check if opening failed
	if ((keyFile == NULL) || (plainFile == NULL)) {
		error("fopen:");
	}

	//initialize two structs: one for the keyFile and one for the plainFile
	struct stat kst;
	struct stat pst;

	//store the keyfile's stats in the kst struct
	stat(argv[2], &kst);
	
	//store the plainfile's stats in the pst struct
	stat(argv[1], &pst);

	//store the sizes of the keyfile and the plainfile in two ints
	int keySize, plainSize, i;
	keySize = (int)kst.st_size; 
	plainSize = (int)pst.st_size;

	//check the sizes:
	if (keySize < plainSize) {
		error("CLIENT: Key is too small");
	}

	//create a string that holds all allowed chars
	char allowedChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";	
	
	//create an iterative string 
	char currChar[2];
	memset(currChar, '\0', 2);
	
	//create 2 strings to hold the amount of data in the key and plain files
	char key[keySize]; 
	char plaintext[plainSize];

	memset(key, '\0', keySize);
	memset(plaintext, '\0', plainSize);
	
	//check each char in the keyFile and the plainFile against allowedChars
	for (i = 0; i < keySize - 1; i++) { //for loop checks the key file 
		currChar[0] = getc(keyFile); //retrieve the current char in the file
		
		//add the currChar to the key
		key[i] = currChar[0];
		
		if (strstr(allowedChars, currChar) == NULL) { //if currChar is inside allowedChars then do not execute
			error("CLIENT: Key has bad characters");
		}
	}
	
	for (i = 0; i < plainSize - 1; i++) { //for loop checks the plainfile 
		currChar[0] = getc(plainFile); //retrieve the current char in the file
		
		//add the currChar to the key
		plaintext[i] = currChar[0];

		if (strstr(allowedChars, currChar) == NULL) { //if currChar is inside allowedChars then do not execute
			error("CLIENT: Plaintext has bad characters");
		}
	}

	fclose(keyFile);
	fclose(plainFile);
	
	// Create a socket
  	socketFD = socket(AF_INET, SOCK_STREAM, 0); 

	if (socketFD < 0){ //error message
       		error("CLIENT: ERROR opening socket");
        } 

     	// Set up the server address struct
       	setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

	// Connect to server
   	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        	error("CLIENT: ERROR connecting");
	}
	
	//check if we really connected with ENC_SERVER or not
	if (checkConnection(socketFD) == 1) { //if the connection is not with enc_server
		perror("CLIENT: Port number not owned by ENC_SERVER");
		exit(2);
	}

	//send the size of the plaintext message to the server
	sendsz(socketFD, plainSize); 

	sendstr(socketFD, plaintext);

	sendsz(socketFD, keySize);

	sendstr(socketFD, key);

	//int pSize = plainSize;

	char* ciphertext = malloc(plainSize*sizeof(char));

	memset(ciphertext, '\0', plainSize*sizeof(char));

	recvstr(socketFD, ciphertext, plainSize);

	printf("%s\n", ciphertext);

	//char* backtext = malloc(plainSize*sizeof(char));

	//memset(backtext, '\0', plainSize*sizeof(char));

	//decrypt(ciphertext, key, backtext);

	//printf("%s\n", backtext);
	
	// Close the socket
	free(ciphertext);
	//free(backtext);
        close(socketFD); 

	return 0;
}



























