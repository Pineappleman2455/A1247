#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <inttypes.h>

#define MAX_THREAD_COUNT 9
#define MAX_TASK_COUNT 3

typedef struct{
	int threadCount;
	pthread_t threadId;
	//pthread_mutex_t g_ThreadMutex;
	//pthread_cond_t g_conditionVar;
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
		g_ThreadArgs[i].threadCount = i;
		g_ThreadArgs[i].threadPri = 10;
		if(i < 3){
			g_ThreadArgs[i].threadPolicy = SCHED_FIFO;
		}
		else if(i >= 3 && i < 6){
			g_ThreadArgs[i].threadPolicy = SCHED_RR;
		}
		else{
			g_ThreadArgs[i].threadPolicy = SCHED_OTHER;
		}
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
  while(longVar < 0xffffffff) longVar++;
}

int main (int argc, char *argv[]){

	InitGlobals();
		for(int i = 0; i < MAX_THREAD_COUNT; i++){
		pthread_create(&(g_ThreadArgs[i].threadId), NULL, *threadFunction, &g_ThreadArgs[i]);
	}
	for(int i =0; i < MAX_THREAD_COUNT; i++){
		pthread_cond_signal(&g_conditionVar[i]);
}
	for(int i = 0; i < MAX_THREAD_COUNT; i++){
		pthread_join(g_ThreadArgs[i].threadId, NULL);
		DisplayThreadArgs(&g_ThreadArgs[i]);
}
	exit(0);
}

void* threadFunction(void *arg)
{
	ThreadArgs* myThreadArg;
	myThreadArg = (ThreadArgs*) arg;
	//struct sched_param param;
	pthread_setschedparam(myThreadArg->threadId, myThreadArg->threadPolicy, myThreadArg->threadPri);
	pthread_mutex_lock( &g_ThreadMutex[myThreadArg->threadCount]);
  pthread_cond_wait( &g_conditionVar[myThreadArg->threadCount], &g_ThreadMutex[myThreadArg->threadCount]);
	pthread_mutex_unlock( &g_ThreadMutex[myThreadArg->threadCount] );

	struct timespec tms;
	clock_gettime(CLOCK_REALTIME, &tms);
	myThreadArg->timeStamp[0] = tms.tv_sec *1000000;
	myThreadArg->timeStamp[0] += tms.tv_nsec/1000;
	if(tms.tv_nsec % 1000 >= 500 ) myThreadArg->timeStamp[0]++;
	int i = 0;
	while(i < 3){
		DoProcess();
		i++;
		clock_gettime(CLOCK_REALTIME, &tms);
		myThreadArg->timeStamp[i+1] = tms.tv_sec *1000000;
		myThreadArg->timeStamp[i+1] += tms.tv_nsec/1000;
		if(tms.tv_nsec % 1000 >= 500 ) myThreadArg->timeStamp[i+1]++;

	}
	//pthread_exit(NULL);
}
