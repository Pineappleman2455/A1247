#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <inttypes.h>

#define MAX_THREAD_COUNT 1
#define MAX_TASK_COUNT 1

typedef struct{
	int threadCount;
	pthread_t threadId;
	int threadPolicy;
	int threadPri;
	long processTime;
	int64_t timeStamp[MAX_TASK_COUNT+1];
	time_t startTime;
	time_t endTime;
} ThreadArgs;

pthread_mutex_t g_ThreadMutex[MAX_THREAD_COUNT];
pthread_cond_t g_conditionVar[MAX_THREAD_COUNT];
ThreadArgs g_ThreadArgs[MAX_THREAD_COUNT];

void InitGlobals(void);
void DisplayThreadSchdAttributes(pthread_t, int, int);
void DisplayThreadArgs(ThreadArgs*);
void DoProcess(void);
void* threadFunction(void *arg);

void InitGlobals(void){
	for(int i = 0; i < MAX_THREAD_COUNT; i++){
		g_ThreadMutex[i] = PTHREAD_MUTEX_INITIALIZER;
		g_conditionVar[i] = PTHREAD_COND_INITIALIZER;
	}
}

void DisplayThreadSchdAttributes( pthread_t threadID, int policy, int priority )
{
  printf("\nDisplayThreadSchdAttributes:\n threadID = 0x%lx\n policy = %s\n priority = %d\n", threadID,
	 (policy == SCHED_FIFO) ? "SCHED_FIFO" :
	 (policy == SCHED_RR)	? "SCHED_RR" :
	 (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???", priority);
}

void DisplayThreadArgs(ThreadArgs* myThreadArg){
  int i,y;
  if(myThreadArg){
    DisplayThreadSchdAttributes(myThreadArg->threadId, myThreadArg->threadPolicy, myThreadArg->threadPri);
    printf(" startTime = %s endTime = %s", ctime(&myThreadArg->startTime), ctime(&myThreadArg->endTime));
    printf(" TimeStamp [%"PRId64"]\n", myThreadArg->timeStamp[0] );
    for(y=1; y<MAX_TASK_COUNT+1; y++){
      printf(" TimeStamp [%"PRId64"] Delta [%"PRId64"]us\n", myThreadArg->timeStamp[y],
	     (myThreadArg->timeStamp[y]-myThreadArg->timeStamp[y-1]));
    }
  }
}

void DoProcess(void){
  unsigned int longVar = 1 ;
  //TODO: Increase the number of fs to take longer
  while(longVar < 0xffff) longVar++;
}

int main (int argc, char *argv[]){
	InitGlobals();
	for(int i = 0; i < MAX_THREAD_COUNT; i++){
		pthread_create(&(g_ThreadArgs[i].threadId), NULL, &threadFunction, &g_ThreadArgs[i]);
	}
	for(int i = 0; i < MAX_THREAD_COUNT; i++){
		threadFunction(&g_ThreadArgs[i]);
	}
	for(int i = 0; i < MAX_THREAD_COUNT; i++){
		pthread_cond_signal( &g_conditionVar[i]);
	}
	for(int i = 0; i < MAX_THREAD_COUNT; i++){
		pthread_join(g_ThreadArgs[i].threadId, NULL);
	}
	for(int i = 0; i< MAX_THREAD_COUNT; i++){
		DisplayThreadArgs(&g_ThreadArgs[i]);
	}
	exit(0);
	/*3.	Assign 3 threads to SCHED_OTHER, another 3 to SCHED_FIFO and another 3 to SCHED_RR
	4.	Signal the condition variable
	5.	Call �pthread_join� to wait on the thread
	6.	Display the stats on the threads*/
}

void* threadFunction(void *arg)
{
	ThreadArgs* myThreadArg;
	myThreadArg = (ThreadArgs*)arg;
	if(myThreadArg->threadCount < 3){
		myThreadArg->threadPolicy = SCHED_FIFO;
	}
	else if(myThreadArg->threadCount >= 3 && myThreadArg->threadCount < 6){
		myThreadArg->threadPolicy = SCHED_RR;
	}
	else{
		myThreadArg->threadPolicy = SCHED_OTHER;
	}
  pthread_setschedparam(pthread_self(), myThreadArg->threadPolicy, &param);
	pthread_mutex_lock( &g_ThreadMutex[myThreadArg->threadCount] );
  pthread_cond_wait( &g_conditionVar[myThreadArg->threadCount], &g_ThreadMutex[myThreadArg->threadCount] );
	struct timespec start, end;
	clock_gettime(CLOCK_REALTIME, &start);
	myThreadArg->startTime = start.tv_sec *1000000;
	myThreadArg->startTime += start.tv_nsec/1000;
	if(start.tv_nsec % 1000 >= 500 ) myThreadArg->startTime++;
	int i = 0;
	while(i < 3){
		DoProcess();
		i++;
	}
	clock_gettime(CLOCK_REALTIME, &end);
	myThreadArg->endTime = end.tv_sec *1000000;
	myThreadArg->endTime += end.tv_nsec/1000;
	if(end.tv_nsec % 1000 >= 500 ) myThreadArg->endTime++;
	pthread_mutex_unlock( &g_ThreadMutex[myThreadArg->threadCount] );
	pthread_exit(NULL);

	/*1.	Typecast the argument to a �ThreadArgs*� variable
	2.	Use the �pthread_setscheduleparam� API to set the thread policy
	3.	Init the Condition variable and associated mutex
	4.	Wait on condition variable
	5.	Once condition variable is signaled, use the �time� function and the �clock_gettime(CLOCK_REALTIME, &tms)� to get timestamp
	6.	Call �DoProcess� to run your task
	7.	Use �time� and �clock_gettime� to find end time.
	8.	You can repeat steps 6 and 7 a few times if you wish*/
}
