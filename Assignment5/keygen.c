//This program creates a key file of specified length, and made up of 27 allowed chars (26 Cap letters and space)
//Written by Gabriel Rodgers in Operating Systems 1.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//store the amount of arguments in argc, and each argument in the char array argv
//argv[0] = executable 
//argv[1] = keylength integer
int main(int argc, char* argv[]) {
	
	//error handling: execute if no keylength value inputted or multiple are inputted
	if (argc < 2) {
		perror("Error: must enter an integer for keylength");
		exit(1);
	}
	
	//create a char array to hold the key (but first, convert the char* keylength int into an actual int)
	int keyLength = atoi(argv[1]);
	char key[keyLength + 1]; 
	memset(key, '\0', keyLength + 1); 

	int val; //holds the current randomly generated int

	int i = 0; //current index

	srand((unsigned) time(NULL)); //generate a random seed for rand()
	
	//generate the key in a while loop
	while (i < keyLength) {
		val = rand() % 100; //create the random value that is roughly between 0 and 100
		
	 	if ((val == 32) || ((65 <= val) && (val <= 90))) { //if the value is within the correct ASCII range
			key[i] = (char) val; //convert the int val to ASCII char and assign it to the key
			i++;
		}
	}
	
	//send the key out to stdout 
	printf("%s\n", key);

 	return 0;
}
