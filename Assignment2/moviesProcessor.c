// If you are not compiling with the gcc option --std=gnu99, then
// uncomment the following line or you might get a compiler warning
#define _GNU_SOURCE

//Code by Gabriel Rodgers
//this code should be able to create files and directories, set permissions to those files and directories, 
//manipulate those files by writing to them, and accept user input and respond accordingly. 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

/* struct for movie information */
struct movie
{
    char *title; // string
    char *year; // int, [1900-2021]
    struct movie *next;
};


/* Parse the current line which is space delimited and create a
*  movie struct with the data in this line
*/
struct movie *createMovie(char *currLine)
{
    struct movie *currMovie = malloc(sizeof(struct movie));

    // For use with strtok_r
    char *saveptr;

    // The first token is the title
    char *token = strtok_r(currLine, ",", &saveptr); //for the title, delimiter should be ","
    currMovie->title = calloc(strlen(token) + 1, sizeof(char)); //create space for the new string to be allocated to title
    strcpy(currMovie->title, token); //copy the data in the file to the title data member

    // The next token is the year
    token = strtok_r(NULL, ",", &saveptr); //for the year, delimiter should be ","
    currMovie->year = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->year, token); //copies the data (second part of the line) to the year data member

    // Set the next node to NULL in the newly created student entry
    currMovie->next = NULL;

    return currMovie;
}

/*
* Return a linked list of movies by parsing data from
* each line of the specified file.
*/
struct movie *processMovieFile(char *filePath)
{
    // Open the specified file for reading only
    FILE *movieFile = fopen(filePath, "r");
    
    //check for if the movieFile does not exist to prevent a segfault
    if (movieFile == NULL) {
	
	perror("fopen");
	exit(EXIT_FAILURE);
    }

    char *currLine = NULL;
    size_t len = 0; //buffer size
    ssize_t nread;  
    char *token;

    // The head of the linked list
    struct movie *head = NULL;
    // The tail of the linked list
    struct movie *tail = NULL;

    // Read the file line by line
    while ((nread = getline(&currLine, &len, movieFile)) != -1)
    {
        // Get a new movie node corresponding to the current line
        struct movie *newNode = createMovie(currLine);

        // Is this the first node in the linked list?

        if (head == NULL)
        {
            // This is the first node in the linked link
            // Set the head and the tail to this node
            head = newNode;
            tail = newNode;
        }
        else
        {
            // This is not the first node.
            // Add this node to the list and advance the tail
            tail->next = newNode;
            tail = newNode;
        }
    }

    free(currLine);
    fclose(movieFile);
    return head;
}


int firstPrompt(){
	
    int firstOption;

    printf("\n1. Select file to process\n");
    printf("2. Exit the program\n");
    printf("\nEnter a choice 1 or 2: ");

    scanf("%d", &firstOption); //scanf for first user input
    

	return firstOption;
}

int secondPrompt(){

    int secondOption;

    printf("\nWhich file do you want to process?\n");
    printf("1. Enter 1 to pick the largest file\n");
    printf("2. Enter 2 to pick the smallest file\n");
    printf("3. Enter 3 to specify the name of a file\n");
    printf("\nEnter a choice from 1 to 3: ");

    scanf("%d", &secondOption); //scanf for second user input

	return secondOption;
}
	

int checkFile(char* fileName) {
	//this function returns a 1 if the fileName starts with movies_ and ends with .csv, and a 0 otherwise
	//this function checks if an inputted file name has a prefix movies_ and a suffix .csv
	//define some variables; use manually allocated char arrays
	char prefix[8] = "movies_";
	char extension[5] = ".csv";

	int i = 0;
	int count = 0;

	int fileNameLen = strlen(fileName);
	int prefixStart = fileNameLen - 4;
	
	//loop for the duration of the fileName's length
	while (i < fileNameLen) {
		
		if (fileName[i] == prefix[i]) { //check for prefix
			count++;
		}

		else if (i >= prefixStart) {
			if (fileName[i] == extension[i-prefixStart]){ //check for extension
				count++;
			}	
		}

		i++;
	}

	if (count == 11) {
		return 1;
	}

	else {
		return 0;
	}
}


int checkTxtFile(char* dirPath, char* year) {

	//this function checks if the year inputted is used inside a .txt file within the dirPath directory
	//this function returns a 1 if the year is already used in a file, and returns a 0 if not or if the 
	//directory is empty
	
	int count = 0;
	
	//initialize some structs and files and directories
	DIR* currentDir = opendir(dirPath);
	struct dirent *currentFile;
	
	if (currentDir > 0) {

		while ((currentFile = readdir(currentDir)) != NULL) {
			
			//check if the year is a substring of any of the files within currentDir
			if (strstr(currentFile->d_name, year) != NULL) { //if year is in the filename then execute
				count++;
			}
		}
	}

closedir(currentDir);
return count; 

}


int optionOne(){
	
	//this function should handle the first option 
	//first task: find the largest file in the directory
	
	DIR* currentDir; //initialize current directory
	
	//initialize 2 structs to hold the current file being checked and the largest file 
	struct dirent *currentFile;
	struct dirent *largestFile;
	
	//initialize 2 structs to hold the data of the current file and the largest file
	struct stat currentAttributes;
	struct stat largestAttributes = {0}; //set all data members to zero

	currentDir = opendir("."); //set the current directory to the directory this program is in and open it
	
	if (currentDir > 0) { //ensure current directory can be opened
		
		//while loop exits when the current file read from the directory is NULL
		while ((currentFile = readdir(currentDir)) != NULL) {
			
			//use the checkFile function to check if the currentFile starts with movies_ and ends with .csv
			if (checkFile(currentFile->d_name) == 1) {
	
				//put the currentFile's data into the currentAttributes struct
				stat(currentFile->d_name, &currentAttributes);

				//compare the largest and the current file sizes
				if ((int)currentAttributes.st_size >= (int)largestAttributes.st_size) { 

					//set the largestfile to be the current file
					largestFile = currentFile;
					
					//set up the largestAttributes struct
					stat(largestFile->d_name, &largestAttributes);
				}
			}
		}
	}
	
	printf("Now processing the chosen file named %s\n", largestFile->d_name);
	//printf("The file specified has %d bits in it\n");
	
	fileProcessor(largestFile->d_name); //call to the file processor function
	
	//close the currentDir
	closedir(currentDir);
	
	return 0;
}



int optionTwo(){

	//this function should handle the second option 
	//first task: find the smallest file in the directory
	
	DIR* currentDir; //initialize current directory
	
	//initialize 2 structs to hold the current file being checked and the smallest file 
	struct dirent *currentFile;
	struct dirent *smallestFile;
	
	//initialize 2 structs to hold the data of the current file and the smallest file
	struct stat currentAttributes;
	struct stat smallestAttributes;

	currentDir = opendir("."); //set the current directory to the directory this program is in and open it

	smallestAttributes.st_size = 10000000; //set the default smallest file size to a large number
	
	if (currentDir > 0) { //ensure current directory can be opened

		//while loop exits when the current file read from the directory is NULL
		while ((currentFile = readdir(currentDir)) != NULL) {
			
			//use the checkFile function to check if the currentFile starts with movies_ and ends with .csv
			if (checkFile(currentFile->d_name) == 1) {
	
				//put the currentFile's data into the currentAttributes struct
				stat(currentFile->d_name, &currentAttributes);

				//compare the smallest and the current file sizes
				if ((int)currentAttributes.st_size <= (int)smallestAttributes.st_size) { 

					//set the smallest file to be the current file
					smallestFile = currentFile;
					
					//set up the smallestAttributes struct
					stat(smallestFile->d_name, &smallestAttributes);
				}
			}
		}
	}
	
	printf("Now processing the chosen file named %s\n", smallestFile->d_name);
	//printf("The file specified has %d bits in it\n");
	
	fileProcessor(smallestFile->d_name); //call to the file processor function
	
	//close the currentDir
	closedir(currentDir);
	
	return 0;

}


int optionThree(){
	
	//NOTE: a lot of this code is from lecture slides 1.6 page 8
	
	//initialize a count variable to help with repeating the 1-3 menu
	int count = 0;

	//initialize a char array to hold the filename (user inputted)
	char fileName[256];
	memset(fileName, '\0', 256);
	
	//initialize a char array to hold the filename (current file being checked)
	char currentFileName[256];
	memset(currentFileName, '\0', 256);

	//this function should handle the third option
	//first task: print a message and take user input for file name:
	printf("Enter the complete file name: ");
	scanf("%s", fileName);

	//first task: find the inputted file in the directory
	DIR* currentDir; //initialize current directory
	
	//initialize a struct to hold the current file being checked
	struct dirent *currentFile;
	
	//initialize a struct to hold the data of the current file
	struct stat fileAttributes;
	
	currentDir = opendir("."); //set the current directory to the directory this program is in and open it
	
	if (currentDir > 0) { //ensure current directory can be opened

		//while loop exits when the current file read from the directory is NULL
		while ((currentFile = readdir(currentDir)) != NULL) {

			//check for string equality
			if (strcmp(currentFile->d_name, fileName)  == 0) {
				
				printf("Now processing the chosen file named %s\n", fileName);

				fileProcessor(fileName); //call to the file processor function

				count = 1;
			}
		}
	}

	if (count == 0) { //print error message for when the file is not found

		printf("The file %s was not found. Try again.\n", fileName);
	}
	
	//close the currentDir
	closedir(currentDir);
	
	return count;
}


int recursiveOption() {
	
	//this function recursively calls itself to handle the thirdOption function
	//this function assumes that the previous action was the user inputting the wrong file for optionThree
	//somewhere the value of file is getting obliterated!!!
	
	int secondOption;
	int count;

	secondOption = secondPrompt();
	
		if (secondOption == 1) {
			optionOne();
		}
			
		else if (secondOption == 2) {
			optionTwo();
		}
			
		else if (secondOption == 3) {

			//call the optionThree function
			count = optionThree();
				
			if (count == 0) { //if the file name inputted was invalid

				while (secondOption == 3) { //keep asking the same menu while secondOption is 3
					secondOption = recursiveOption();
				}
			}
		}

		else {
			printf("You entered an incorrect choice. Try again.\n");
			
		}

	return secondOption;	
}


//this function allows the fileProcessor function to call this multiple times in the case where the random
//integer generated is already used in one of the directories
int randomizer(int randomValue){

	while (randomValue >= 100000) { //use while loop to call random() until a value below 100000 is found
		randomValue = random();
	}
		
	return randomValue;
}


int fileProcessor(char* fileName) {
	
	//this function should take in a filename and return 0. 
	//this function takes a file, and processes it. details on how the processing is done is interpersed. 
	//the first part of the processing is creating a new directory called rodgerga.movies.random (random = [0-99999])
	//this directory must be set to rwxr-x--- permissions. 
	
	//first task: creating a new directory to put the files in!
	
	//initialize the random number between 0 and 99999
	int randomValue;

	//initialize the first node of the movie LL
	struct movie* head;

	//initialize the current directory and a file within that directory
	DIR* currentDir = opendir(".");
	struct dirent *currentFile;

	int count = 0;
	int i = 0;

	//while loop reads through the entries of the directory and finds the number of files in the directory
	while ((currentFile = readdir(currentDir)) != NULL) {
		count++;	
	}
	//this ensures that there are no duplicate directories
	for (i; i<=count; i++) {
		randomValue = randomizer(100000);
	}

	//initialize a string to hold the random value int
	char randomValStr[6];
	memset(randomValStr, '\0', 6);
	
	//convert the randomValue to string
	sprintf(randomValStr, "%d", randomValue);

	//printf("%s\n", randomValStr);
	char dirName[23] = "rodgerga.movies."; 

	strcat(dirName, randomValStr); //after this, the new full directory name is stored in dirName

	//prevent any errors in creating the new directory 
	if (mkdir(dirName, 0750) == -1) {
		
		perror("mkdir");
		exit(EXIT_FAILURE);
	}

	printf("Created directory with name %s\n", dirName);

	closedir(currentDir);

	//the second part of the processing is parsing the data in the file called fileName to find each movie for each year.
	//my first idea is to create an LL of movies, with each movie struct containing its year, its title, and a pointer to
	//the next movie struct
	//this will be done with the function processMovieFile
	
	head = processMovieFile(fileName);
	head = head->next; //do this to remove the header
	
	//the third part of the processing is creating a file for each year YYYY that movies were released in, called 
	//YYYY.txt
	//YYYY.txt should be put into the rodgerga.movies.random directory for that given fileName
	
	//the fourth part of the processing is writing to each YYYY.txt:
	//each line in YYYY.txt should have a separate movie title for each movie that was released in that year. 
	
	//my thought process: go through the LL of movies in a while loop until hitting the end, starting with 
	//the second node (to ignore the header). For each movie struct, check if its year data member is used in 
	//a file inside the parent directory. 
	
	//create two strings to concatenate together:
	char extension[5] = ".txt";
	char year[5] = "YYYY";

	//create a string to hold the file name
	char txtFile[33];

	//initialize a file
	FILE* yearTxt;

	//loop through the movie LL
	while (head != NULL){
		
		memset(txtFile, '\0', 33); //reset the txtFile 

		//create the txtFile name given the movie year
		strcat(txtFile, dirName);
		strcat(txtFile, "/");
		strcat(txtFile, head->year);
		strcat(txtFile, extension);
		//by the end of this, txtFile should look like this:
		//rodgerga.movies.rand#/YYYY.txt

		//filter for case where the current movie's year is not in the directory 
		if (checkTxtFile(dirName, head->year) == 0) { //execute if year is not in the directory
			
			//create the new text file with fopen() then change permissions using chmod()
			yearTxt = fopen(txtFile, "w+");
			chmod(txtFile, 0640);

			//now to write to that newly created file:
			fprintf(yearTxt, "%s\n", head->title);
			
			//close the file
			fclose(yearTxt);
		}
		
		else { //execute if year is in the directory
			
			//open the text file with fopen() in append mode
			yearTxt = fopen(txtFile, "a+");
			
			//now to write to that opened file:
			fprintf(yearTxt, "%s\n", head->title);
			
			//close the file
			fclose(yearTxt);
		}

		head = head->next;
	}

	return 0;

}


int main()
{
    //initialize three integers
    int firstOption = firstPrompt();
    int secondOption;
    int count;
	
    	while (firstOption != 2) { //loops for as long as the user input is not 2
		
		if (firstOption == 1) { //this if statement handles all second options
			
			secondOption = secondPrompt(); //asks the user for a choice again

			if (secondOption == 1) {
				optionOne();	
			}
			
			else if (secondOption == 2) {
				optionTwo();
			}
			
			else if (secondOption == 3) {

				//call the optionThree function
				count = optionThree();
				
				if (count == 0) { //if the file name inputted was invalid
 					
					secondOption = recursiveOption(); //call the recursive function
				}
			}

			else {
				printf("You entered an incorrect choice. Try again.\n");
		
			}
			
			firstOption = firstPrompt(); //asks the user for a choice again
		}

		else if (firstOption == 2) { 
			return 0;		

		}
		
		else {
			
			printf("You entered an incorrect choice. Try again.\n");
			firstOption = firstPrompt();
		}	
	}

	return 0;	

}
		
