#include<stdio.h>
#include<stdlib.h>
#include <sys/time.h>
#include <pthread.h>

typedef struct state {
	int *resource;
	int *available;
	int **max;
	int **allocation;
	int **need;
} State;

int isSafeState(State*);
State *initialise_State(void);
State *copy_State(void);
int do_request(int, int*, State*);
void print_avaialble(void);
void print_allocated(void);

// Global variables
int m, n;
State *s;

// Mutex for access to state.
pthread_mutex_t state_mutex;


/* Random sleep function */
void Sleep(float wait_time_ms)
{
	// add randomness
	wait_time_ms = ((float)rand())*wait_time_ms / (float)RAND_MAX;
	usleep((int) (wait_time_ms * 1e3f)); // convert from ms to us
}

/* Allocate resources in request for process i, only if it 
   results in a safe state and return 1, else return 0 */
int resource_request(int i, int *request)
{
	pthread_mutex_lock(&state_mutex);
	int r;
	State *copy = copy_State();
	if (do_request(i, request, copy)) {
		if (isSafeState(copy)) {
			do_request(i, request, s);
			r = 1;
		} 
	}
	pthread_mutex_unlock(&state_mutex);
	if (r) print_available();
	return r;
}

/* Apply request onto given state */
int do_request(int i, int *request, State *state){
	int j;
	for(j = 0; j < n; j++){
		if(request[j] > state->available[j]) return 0;
		state->available[j] -= request[j];
		state->need[i][j] -= request[j];
		state->allocation[i][j] += request[j];
	}
	return 1;
}

/* Release the resources in request for process i */
void resource_release(int i, int *request) {
	pthread_mutex_lock(&state_mutex);
	int j;
	for(j = 0; j < n; j++){
		s->available[j] += request[j];
		s->need[i][j] += request[j];
		s->allocation[i][j] -= request[j];
	}
	print_available();
	pthread_mutex_unlock(&state_mutex);
}

/* Generate a request vector */
void generate_request(int i, int *request)
{
	int j, sum = 0;
	while (!sum) {
		for (j = 0;j < n; j++) {
			request[j] = s->need[i][j] * ((double)rand())/ (double)RAND_MAX;
			sum += request[j];
		}
	}
	printf("Process %d: Requesting resources.\n",i);
}

/* Generate a release vector */
void generate_release(int i, int *request)
{
	int j, sum = 0;
	while (!sum) {
		for (j = 0;j < n; j++) {
			request[j] = s->allocation[i][j] * ((double)rand())/ (double)RAND_MAX;
			sum += request[j];
		}
	}
	printf("Process %d: Releasing resources.\n",i);
}

/* Threads starts here */
void *process_thread(void *param)
{
	/* Process number */
	int i = (int) (long) param, j;
	/* Allocate request vector */
	int *request = malloc(n*sizeof(int));
	while (1) {
		/* Generate request */
		generate_request(i, request);
		while (!resource_request(i, request)) {
			/* Wait */
			Sleep(100);
		}
		/* Generate release */
		generate_release(i, request);
		/* Release resources */
		resource_release(i, request);
		/* Wait */
		Sleep(1000);
	}
	free(request);
}

/*allocate memory for a matrix*/
int **allocate_int_matrix(int r, int c){

	int *mem = (int *) malloc(m * n * sizeof(int));
	int **mat = (int **) malloc(m * sizeof(int *));

	if (mem != NULL && mat != NULL){
		int i;
		for (i = 0; i < m; i++){
			mat[i] = &mem[i * n];
		}	
	} else {
		printf("Out of memory \n"); exit(-1);
	}
	return mat;
}

int isSafeState(State *state){
	int j, i, safe;
	for(i = 0; i < m; i++){
		for(j = 0; j < n; j++){
			if(state->need[i][j] > state->available[j]) {
				safe = 0;	
			}
		}
		if (safe) return safe;
	}
	return safe;
}

State *copy_State(void){
	State *copy = initialise_State();
	int i, j;
	for(i = 0; i < m; i++){
		for(j = 0; j < n; j++){
			copy->allocation[i][j] = s->allocation[i][j];
			copy->max[i][j] = s->max[i][j];
			copy->need[i][j] = s->need[i][j];
		}
		copy->resource[i] = s->resource[i];
		copy->available[i] = s->available[i];
	}
	return copy;
}

State *initialise_State(void){
	State *st = (State *)  malloc(sizeof *s);
	st->resource = (int *)malloc(n * sizeof(int));
	st->available = (int *)malloc(n * sizeof(int));
	st->allocation = allocate_int_matrix(m, n);
	st->max = allocate_int_matrix(m, n);
	st->need = allocate_int_matrix(m, n);
	return st;
}

void print_available(void){
	printf("Availability vector:\n");
	int i, j;
	for(i = 0; i < n; i++)
		printf("R%d ", i+1);
	printf("\n");
	for(j = 0; j < n; j++)
		printf("%d  ",s->available[j]);
	printf("\n");
	print_allocated();
}

void print_allocated(void){
	printf("ALlocated matrix:\n");
	int i, j;
	for(i = 0; i < n; i++)
		printf("R%d ", i+1);
	printf("\n");
	for(i = 0; i < m; i++) {
		for(j = 0; j < n; j++)
			printf("%d  ",s->allocation[i][j]);
		printf("\n");
	}
}


int main(int argc, char* argv[])
{
	/* Get size of current state as input */
	int i, j;
	printf("Number of processes: ");
	scanf("%d", &m);
	printf("Number of resources: ");
	scanf("%d", &n);

	s = initialise_State();

	/* Allocate memory for state */
	if (s == NULL) { printf("\nYou need to allocate memory for the state!\n"); exit(0); };

	/* Get current state as input */
	printf("Resource vector: ");
	for(i = 0; i < n; i++)
		scanf("%d", &s->resource[i]);
	printf("Enter max matrix: ");
	for(i = 0;i < m; i++)
		for(j = 0;j < n; j++)
			scanf("%d", &s->max[i][j]);
	printf("Enter allocation matrix: ");
	for(i = 0; i < m; i++)
		for(j = 0; j < n; j++) {
			scanf("%d", &s->allocation[i][j]);
		}
	printf("\n");

	/* Calcuate the need matrix */
	for(i = 0; i < m; i++)
		for(j = 0; j < n; j++)
			s->need[i][j] = s->max[i][j]-s->allocation[i][j];

	/* Calcuate the availability vector */
	for(j = 0; j < n; j++) {
		int sum = 0;
		for(i = 0; i < m; i++)
			sum += s->allocation[i][j];
		s->available[j] = s->resource[j] - sum;
	}

	/* Output need matrix and availability vector */
	printf("Need matrix:\n");
	for(i = 0; i < n; i++)
		printf("R%d ", i+1);
	printf("\n");
	for(i = 0; i < m; i++) {
		for(j = 0; j < n; j++)
			printf("%d  ",s->need[i][j]);
		printf("\n");
	}
	printf("Availability vector:\n");
	for(i = 0; i < n; i++)
		printf("R%d ", i+1);
	printf("\n");
	for(j = 0; j < n; j++)
		printf("%d  ",s->available[j]);
	printf("\n");

	/* If initial state is unsafe then terminate with error */

	/* Seed the random number generator */
	struct timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);

	/* Create m threads */
	pthread_t *tid;
	/* Check that initial state is safe */
	if (isSafeState(s)){
		tid = malloc(m*sizeof(pthread_t));
		for (i = 0; i < m; i++)
			pthread_create(&tid[i], NULL, process_thread, (void *) (long) i);
	}
	/* Wait for threads to finish */
	pthread_exit(0);
	/* Free state memory */
	free(tid);
}


