This file contains instructions on how to compile my code using gcc to create an executable file called movies (and then run that)
Do the following in the os1.engr.oregonstate.edu server:
Run the following commands with the following assumptions:
 - $ is the end of your path
 - the movies.c file is in the same directory as the test file (test_file)
 - you are in the same directory as the test and movies.c files
 - (otherwise, the path will need to be specified)
$ gcc -o movies ./movies.c
$ ./movies test_file


^the first command compiles and the second command runs