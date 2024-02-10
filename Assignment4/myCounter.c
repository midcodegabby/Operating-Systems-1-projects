//Program Title:  Multi-threaded Producer Consumer Counter
//made by Gabriel Rodgers
//This code will use two threads, and will communicate inter-threads
// - Producer Thread (original)
// - Consumer Thread (created)

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

//initialize the counting variables
int myCount = 0; 

//this int will allow coordination between the two threads: 
int possession = 0;
//possession = 0: allow consumer to increment
//possession = 1: allow producer to increment 

//initialize the condition variables
pthread_cond_t myCond1 = PTHREAD_COND_INITIALIZER; //if True, allow producer to increment myCount
pthread_cond_t myCond2 = PTHREAD_COND_INITIALIZER; //if True, allow consumer to decrement myCount

//initialize the mutex
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER; 

//create a function for the consumer thread to operate in
void *consumerFunction() {
	
	while (myCount < 10) { //end the while loop when myCount reaches 10
		
		//lock the mutex before checking if myCond2 is T/F
		pthread_mutex_lock(&myMutex);

		if (myCount == 10) {
			return NULL;
		}

		printf("CONSUMER: myMutex locked\n"); //myMutex lock message	
		
		//use a while loop to wait for myCond2 to evaluate to True
		while (possession == 1) {
			printf("CONSUMER: waiting on myCond2\n"); //wait message
			pthread_cond_wait(&myCond2, &myMutex); 
		}
		
		//prevent this code from running if myCount reaches 10
		if (myCount < 10) {
			//when the thread gets to this point, this means that we can increment the count
			printf("myCount: %d -> %d\n", myCount, myCount+1); //myCount change message
			myCount = myCount + 1;

			//unlock the mutex and then signal myCond1 to be True
			printf("CONSUMER: myMutex unlocked\n"); //myMutex unlock message	
			pthread_mutex_unlock(&myMutex);
			
			possession = 1;

			printf("CONSUMER: signaling myCond1\n"); fflush(stdout); //myCond1 signaling message
			pthread_cond_signal(&myCond1); //signal that there is space available
		}
	}
	return NULL;
}

int main() {
	
	//print out the starting message
	printf("PROGRAM START\n");

	//create the thread and tell it to go into the consumer function
	pthread_t consumer;
	pthread_create(&consumer, NULL, consumerFunction, NULL);

	//print out the consumer thread created message
	printf("CONSUMER THREAD CREATED\n");

	while (myCount < 10) { //loop until myCount reaches 10
		//lock the mutex before checking if myCond1 is T/F
		pthread_mutex_lock(&myMutex);
		printf("PRODUCER: myMutex locked\n"); //myMutex lock message	

		while (possession == 0) {
			printf("PRODUCER: waiting on myCond1\n"); //wait message
			pthread_cond_wait(&myCond1, &myMutex); 
		}

		//when the thread gets to this point, this means that we can increment the count
		printf("myCount: %d -> %d\n", myCount, myCount+1); //myCount change message
		myCount = myCount + 1;

		//unlock the mutex and then signal myCond2 to be True
		printf("PRODUCER: myMutex unlocked\n"); //myMutex unlock message	
		pthread_mutex_unlock(&myMutex);
		
		possession = 0;

		printf("PRODUCER: signaling myCond2\n"); fflush(stdout); //myCond2 signaling message
		pthread_cond_signal(&myCond2); //signal that there is data
		
		if (myCount == 10) {
			break;
		}
	}
	
	pthread_join(consumer, NULL); //wait for the consumer thread to terminate

	printf("PROGRAM END\n"); //print out the final message
	return 0;
}

