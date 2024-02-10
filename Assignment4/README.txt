This file contains instructions on how to compile my code using gcc to create an executable file called myCounter.
Run the following commands with the following assumptions:
 - $ is the line indicator thing in bash 
 - the myCounter.c file is in the same directory as the test file
 - you are in the same directory as the myCounter.c file
 - (otherwise, the path will need to be specified)
 - please note that some runs might have the "thread: myMutex locked" message not directly before the myCount change, but I think this is fine
   since that is due to the functionality of the pthread_cond_wait function
$ gcc -lpthread -o myCounter myCounter.c
$ ./myCounter
