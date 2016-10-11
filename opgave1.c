#include <float.h>
#include <math.h>
#include <pthread.h> 
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS 4

double sum; /* this data is shared by the thread(s) */
void *runner(void *param); /* threads call this function */
pthread_mutex_t mutex = {0};

struct arg_struct {
    int low;
    int high;
};

int main(int argc, char *argv[])
{
	pthread_t tid[NUM_THREADS]; /* the thread identifier */
	pthread_attr_t attr; /* set of thread attributes */
	if (argc != 2) {
		fprintf(stderr,"usage: a.out <integer value>\n"); return -1;
	}
	if (atoi(argv[1]) < 0) {
		fprintf(stderr,"%d must be >= 0\n",atoi(argv[1])); return -1; 
	}
	/* get the default attributes */ 
	pthread_attr_init(&attr);
	
	int n = atoi (argv[1]);
	int t;
	struct arg_struct args[NUM_THREADS];
	for (t = 0; t < NUM_THREADS; t++){
		args[t].low = (t * (n/NUM_THREADS)) + 1;
		args[t].high = (t+1) * (n/NUM_THREADS);
		pthread_create(&tid[t],&attr,runner,(void*) &args[t]); /* wait for the thread to exit */
	}

	//struct used to give lower and higher bound of for loop
	//wait for all threads to finish before printing sum
	int x;
	for(x = 0; x < NUM_THREADS; x++){
		pthread_join(tid[x],NULL); 
	}

	printf("sum = %f\n",sum);
}

/* The thread will begin control in this function */ 
void *runner(void *param)
{
	struct arg_struct *args = param;
	double subSum = 0.0; //using subSum to avoid constant locking and unlocking
	for (int i = args -> low; i <= args -> high; i++)
		subSum += sqrt((double)i);

	pthread_mutex_lock(&mutex);
	sum += subSum;
	pthread_mutex_unlock(&mutex);
	pthread_exit(0);
}
