#include <unistd.h> 
#include <string.h>
#include <sys/time.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "list.h"

#define BUFFER_SIZE 5
#define MAX_PRODUCERS 3
#define MAX_CONSUMERS 3
#define NUMBER_OF_PRODUCTS 10

int consumeMore(void);
int produceMore(void);
void* consume(void*);
void* produce(void*);
void Sleep(float); 
sem_t empty, full; 
pthread_mutex_t mutex, prodMutex, conMutex; 
List *buffer;
int produced;
int consumed;

int main(){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);

	sem_init(&empty, 0, BUFFER_SIZE);
	sem_init(&full, 0, 0);
	pthread_mutex_init(&prodMutex, NULL);
	pthread_mutex_init(&conMutex, NULL);
	pthread_mutex_init(&mutex, NULL);
	produced = 0;
	consumed = 0;
	buffer = list_new();

	pthread_t tidPro[MAX_PRODUCERS]; /* the thread identifier */
	pthread_t tidCon[MAX_CONSUMERS]; /* the thread identifier */
	pthread_attr_t attr; /* set of thread attributes */
	pthread_attr_init(&attr);


	int i;
	for(i = 0; i < MAX_PRODUCERS; i++){
		pthread_create(&tidPro[i],&attr,produce,(void *)(intptr_t)i);
	}

	int j;
	for(j = 0; j < MAX_CONSUMERS; j++){
		pthread_create(&tidCon[j],&attr,consume,(void *)(intptr_t)j); 
	}
	int x;
	for(x = 0; x < MAX_PRODUCERS; x++){
		pthread_join(tidPro[x],NULL); 
	}
	int y;
	for(y = 0; y < MAX_CONSUMERS; y++){
		pthread_join(tidCon[y],NULL); 
	}
	exit(0);
}


void* produce(void *vp){
	int i = (intptr_t)vp;
	Node *n;
	int item;

	while(item = produceMore()){
		Sleep(1000);

		sem_wait(&empty);
		pthread_mutex_lock(&mutex);

		char number[15];
		sprintf(number, "%d", item);
		node_new_str(number);
		list_add(buffer, n);

		pthread_mutex_unlock(&mutex);
		sem_post(&full);

		printf("Producer_%d produced Item_%s. Items in buffer: %d (out of %d). \n", i, number, buffer->len, BUFFER_SIZE);
	}
	pthread_exit(0);
}

void* consume(void *vp){
	int i = (intptr_t)vp;
	Node *n;
	int item;

	while(item = consumeMore()){
		Sleep(1000);

		sem_wait(&full);
		pthread_mutex_lock(&mutex);

		n = list_remove(buffer);

		pthread_mutex_unlock(&mutex);
		sem_post(&empty);

		printf("Consumer_%d consumed Item_%s. Items in buffer: %d (out of %d). \n", i, (char *)n->elm, buffer->len, BUFFER_SIZE);
	}
	pthread_exit(0);
}

/* Random sleep function */
void Sleep(float wait_time_ms)
{
	wait_time_ms = ((float)rand())*wait_time_ms / (float)RAND_MAX;
	usleep((int) (wait_time_ms * 1e3f)); // convert from ms to us
}

//atomic handling of number of products consumed
int consumeMore(void){
	int r = 0;
	pthread_mutex_lock(&conMutex);
	if (consumed < NUMBER_OF_PRODUCTS) {
		r = ++consumed;
	}
	pthread_mutex_unlock(&conMutex);
	return r;
}

//atomic handling of number of products produced
int produceMore(void){
	int r = 0;
	pthread_mutex_lock(&prodMutex);
	if (produced < NUMBER_OF_PRODUCTS) {
		r = ++produced;
	}
	pthread_mutex_unlock(&prodMutex);
	return r;
}
