/******************************************************************************
  main.c

  Implementation of a simple FIFO buffer as a linked list defined in list.h.

 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "list.h"
#define NUM_THREADS 4

// FIFO list;
List *fifo;
void* addElements(void *);

int main(int argc, char* argv[])
{
	pthread_t tid[NUM_THREADS]; /* the thread identifier */
	pthread_attr_t attr; /* set of thread attributes */
	pthread_attr_init(&attr);

	fifo = list_new();

	int t;
	for (t = 0; t < NUM_THREADS; t++){
		pthread_create(&tid[t],&attr,addElements, NULL); /* wait for the thread to exit */
	}

	int x;
	for(x = 0; x < NUM_THREADS; x++){
		pthread_join(tid[x],NULL); 
	}
	printf("number: %d \n", fifo->len);

	return 0;
}

void* addElements(void *vp){
	int i;
	for(i = 0; i < 1000000; i++){
		Node *n = node_new_str("xD");
		list_add(fifo, n);
	}
}
