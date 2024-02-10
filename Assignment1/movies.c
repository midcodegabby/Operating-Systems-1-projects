// If you are not compiling with the gcc option --std=gnu99, then
// uncomment the following line or you might get a compiler warning
#define _GNU_SOURCE

//This code should be able to take a text file with movie data, create a linked list of structs that contain
//that movie data, and then print the data out. 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/* struct for movie information */
struct movie
{
    char *title; // string
    char *year; // int, [1900-2021]
    char *languages; // only has to be a string of languages separated by semi colons
    char *rating; // rating value, 1-10 integer or double
    struct movie *next;
};


/* Just brainstorming how to extract values from the above data members of the struct:
 * title: just use &title to get the title of the movie.
 * year: first we have to convert the string *year to an integer. To do this, we have to use the function atoi-
 * -then we compare that newly converted year data member to the input provided by user.
 * languages: just use a string comparison between the user inputted language and &language to see if the language-
 * -can be found within the languages data member. or we can just use strtok to do stuff; use strstr() to see if-
 * -a string is within another string, and strcmp() to compare two strings for equality.
 * rating: first we have to do some comparisons: 
 * --if rating has a "." in it, then we convert the string to a double using the function strtod.
 * --else convert the string to an integer using atoi.
 * -from there we can just compare values in the linked list to find the highest rating within the LL.
 * 
 */



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

    // The next token is the languages
    token = strtok_r(NULL, ",", &saveptr); //this will set the token to be a string containing [languages].
    currMovie->languages = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->languages, token);

    // The last token is the rating
    token = strtok_r(NULL, "\r\n", &saveptr); //use \r\n to remove the carriage return from the token (this caused a lot of issues for me)
    currMovie->rating = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->rating, token);

    // Set the next node to NULL in the newly created student entry
    currMovie->next = NULL;

    return currMovie;
}

/*
* Return a linked list of movies by parsing data from
* each line of the specified file.
*/
struct movie *processFile(char *filePath)
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

/*
* Print data for the given movie (this function was useful in the start)
*/
void printMovie(struct movie* aMovie){
  printf("%s %s %s %s\n", aMovie->title,
		  aMovie->year,
		  aMovie->languages,
		  aMovie->rating);
}

/*
* Print the linked list of movies (repurposed to return the number of movies in the LL)
*/
int printMovieList(struct movie *list)
{
	
	//create a variable num_movies to hold the number of movies
	int num_movies = 0;

    while (list != NULL)
    {
        //printMovie(list);
        list = list->next;
	num_movies = num_movies + 1;
   }
    
	return num_movies;
}

int prompts(){

    //This function takes no parameters, prints out 4 prompts, and returns the integer scanned in by the user.
    printf("\n1. Show movies released in the specified year\n");
    printf("2. Show highest rated movie for each year\n");
    printf("3. Show the title and year of release of all movies in a specific language\n");
    printf("4. Exit from the program\n");
    printf("\nEnter a choice from 1 to 4: ");
    
    //initalize an integer to hold the user's choice
    int option;
    scanf("%d", &option);

return option;
}



//From this moment on I have to start working on implementing the functions; the base printout functionality is there.

/* 
 * The first function to work on is a way to show movies released in a certain year.
 * functionality: 
 * - show this prompt: "1. Show movies released in the specified year"
 * - if the user chooses 1, then show this prompt: "Enter the year for which you want to see the movies: "
 * - from that prompt, collect user input in the form of a 4 digit integer, [1900-2021]
 * - if there are no movies with released in that year, then return the prompt, "No data about movies released in the year [x]"
 * - else: return the titles of the movies released in that year, one title per line
*/
int moviesByYear(struct movie* head){
	

    int year;
    int movies = 0;
    int movie_year;

    printf("Enter the year for which you want to see movies: ");
    scanf("%d", &year);
	
    //while loop to iterate through the movie LL
    while (head != NULL)
    {
        //convert the current movie's year data member from string to int using the function atoi
        movie_year = atoi(head->year);

	//look through the LL of movies for movies that have the same date as the user input year
	if (movie_year == year){
		
		printf("%s\n", head->title);
		movies = movies + 1;
   }

	head = head->next;
}

     if (movies == 0){
	
	printf("No data about movies released in the year %d\n", year);

}
     return 0;

}


//This function will print out a list of the most highly rated movies in each year
int moviesByRating(struct movie* aMovie, int num_movies){

    char* endptr;

    //create two double values that will hold the high and current movie ratings
    double high_rating;
    double current_rating;

    int storage = (4*num_movies) + 1;

    //initialize struct movie* temporary pointers 
    struct movie* high = NULL;
    struct movie* current = NULL;

    //initialize a string to hold the years checked already
    char* year_string;
    year_string = malloc(storage);
    memset(year_string, '\0', storage);
    
    //while loop to iterate through the movie LL
    while (aMovie != NULL){

	current = aMovie;
	high = aMovie;
		
	if (strstr(year_string, aMovie->year) == NULL){  //if aMovie->year is inside the year_string then do not execute
		
		while (current != NULL){

			if (strcmp(current->year, high->year) == 0){ //if current->year is equal to high->year then execute
				
				//time to compare ratings!
				high_rating = strtod(high->rating, &endptr);
				current_rating = strtod(current->rating, &endptr);
				
				if (current_rating > high_rating){
					
					high = current;
				}
			}

				current = current->next; //move on to the next movie

		}
		
		//use .1f and strtod to get one decimal place on all rating values printed out
		printf("%s %.1f %s\n", high->year, strtod(high->rating, &endptr), high->title);
		strcat(year_string, high->year);

	}
	 
	aMovie = aMovie->next;
}
     

     free(year_string); 
     return 0;

}

int moviesByLanguage(struct movie* head){

    int movies = 0;
    char language[21]; //assuming the maximum length of the language inputted is 20 characters
    memset(language, '\0', 21); //set all 21 chars in the language string to null terminators

    printf("Enter the language for which you want to see movies: ");
    scanf("%s", language);

    //The idea here is to loop through the movie LL and then print out year and title of each movie that has the 
    //inputted language string in its languages string.
    while (head != NULL) {
	
	//compare strings using strstr
	if (strstr(head->languages, language) != NULL){  //if language is in head->languages then execute
		
		printf("%s %s\n", head->year, head->title);
		movies = movies + 1;

	}
 
	head = head->next;   
    }
	
    if (movies == 0) {
	printf("No data about movies released in %s\n", language);
}

    return 0;

}

/*
*   Process the file provided as an argument to the program to
*   create a linked list of movie structs and print out the list.
*   Compile the program as follows:
*       gcc --std=gnu99 -o movies main.c
*/

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("You must provide the name of the file to process\n");
        printf("Example usage: ./movies movies_sample_1.txt\n");
        return EXIT_FAILURE;
    }
    
    struct movie *list = processFile(argv[1]);
    int movies = printMovieList(list) - 1;
    
    // print out prompts for the user to respond to:
    printf("Processed file %s and parsed data for %d movies\n", argv[1], movies); 
    int option = prompts(); // call the prompts function and assign the scanned user value to the option variable

	while (option != 4){
		
		if (option == 1){
			moviesByYear(list);
			option = prompts();
}
		else if (option == 2){
			moviesByRating(list->next, movies);
			option = prompts();
}
		else if (option == 3){
			moviesByLanguage(list);
			option = prompts();
}
		else{
			printf("You entered an incorrect choice. Try again.\n");
			option = prompts();
   }
}


    return EXIT_SUCCESS;


}
