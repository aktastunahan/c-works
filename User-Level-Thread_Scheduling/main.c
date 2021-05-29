#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <unistd.h>

#include <string.h>

#define STACK_SIZE 4096
#define ARRAY_SIZE 5
#define EMPTY 0
#define RUNNING 1
#define READY 2
#define FINISHED 3
#define P_WF 0
#define SRTF 1
#define SWITCH_PERIODE_SEC 2.05

typedef struct ThreadInfo {
    ucontext_t context;
    int state;
    int id;
    int count;
    int count_max;
    int tickets;
} ThreadInfo;

typedef struct ThreadHistory {
	int id;
	struct ThreadHistory *next;
} ThreadHistory;

// ************************** //
// GLOBAL VARIABLES
// ************************** //
ThreadInfo threads[ARRAY_SIZE];
ThreadHistory *finished_threads;
int total_share = 0;
int cur_idx = 0;
int fun_sel = -1;
int last_handled_thread_id = -1;
void exitThread(ThreadInfo* thread);

// print information about threads avaliable on the ThreadInfo array
void printInfo(){
  int i;
	printf("\n");
	printf("running>");
	for(i=0; i<ARRAY_SIZE; i++)
		if(threads[i].state == RUNNING){
			printf("T%d", threads[i].id);
			break;
		}

	printf("\tready>");
	for(i=0; i<ARRAY_SIZE; i++)
		if(threads[i].state == READY)
			printf("T%d, ", threads[i].id);
		
	printf("\tfinished>");
	
	ThreadHistory *iter = finished_threads;
	
	while(iter != NULL){
		printf("T%d, ", iter->id);
		iter = iter->next;
	}
	printf("\n");
}

// check if all threads is finished in the array
int isAllFinished(){
	int i;
	for(i=1; i<ARRAY_SIZE; i++)
		if( (threads[i].state == RUNNING || threads[i].state == READY) )
			return 0;
	return 1;
}

// ************************** //
// THREAD FUNCTION
// ************************** //
void threadFun(ThreadInfo *thread) {
	int i;
	while((thread->count) <= (thread->count_max)){
		for(i=0; i<thread->id-1; i++)
			printf("\t");
		printf("%d\n", (thread->count)++);
		sleep(1);
	}
 
	alarm(0);
    // end state
	exitThread(thread);
}

// ************************** //
// CONTEXT SWITCH FUNCTIONS
// ************************** //

// initializes all global data structures for the thread
void initializeThread(int idx, int id, int count_max) {
    threads[idx].state = READY;
    threads[idx].id = id;
    threads[idx].count = 1;
    threads[idx].count_max = count_max;
    threads[idx].tickets = count_max;
}

// creates a new thread
int createThread(int idx, void *function) {
    // create stack
    void *stack;
    getcontext(&(threads[idx].context));
    
    // allocate memory for the stack
    stack = malloc(STACK_SIZE);
    if(stack == NULL){
      printf("Memory error (malloc)\n");
      return -1;
    }
    
    // initialize ucontext
    threads[idx].context.uc_stack.ss_sp = stack;
    threads[idx].context.uc_stack.ss_size = STACK_SIZE;
    threads[idx].context.uc_stack.ss_flags = 0;

    // setup the function
    makecontext(&(threads[idx].context), function, 2, &threads[idx]);

    return 0;
}

// switches control from main thread to one of the threads in the array
void runThread(int new_idx) {
	
	// state updates
	if(threads[cur_idx].state == RUNNING)
		threads[cur_idx].state = READY;
    threads[new_idx].state = RUNNING;
	
	// if the new thread is not the main, print thread information
	if(new_idx != 0)
		printInfo();
	
    // switch to next thread
	int old_idx = cur_idx;
	cur_idx = new_idx;
	alarm(SWITCH_PERIODE_SEC);
	swapcontext(&(threads[old_idx].context), &(threads[new_idx].context));
}

// removes the thread from the thread array
void exitThread(ThreadInfo* thread) {
	thread->state = FINISHED;
	free((thread->context).uc_stack.ss_sp);
	(thread->context).uc_stack.ss_size = 0;
	if(finished_threads == NULL){
		finished_threads = (ThreadHistory*) malloc(sizeof(ThreadHistory));
		finished_threads->id = thread->id;
		finished_threads->next = NULL;
	}
	else{
		ThreadHistory *iter = finished_threads;
		while(iter->next != NULL)
			iter = iter->next;
		iter->next = (ThreadHistory*) malloc(sizeof(ThreadHistory));
		iter = iter->next;
		iter->id = thread->id;
		iter->next = NULL;
	}
	if(thread->id != 0);
		runThread(0);
}

// Returns an empty index in ThreadInfo array. Returns -1 if no empty position
int getEmptyIndex(){
	int i;
	for(i=1; i<ARRAY_SIZE; i++)
		if((threads[i].state == EMPTY) || (threads[i].state == FINISHED))
			return i;
	return -1;
}

// calculate number of tickets.
int getNumTickets(){
  int num_tickets = 0, i;
	for(i=1; i<ARRAY_SIZE; i++)
    if((threads[i].state == RUNNING) || (threads[i].state == READY))
	    num_tickets += threads[i].tickets;
  return num_tickets;
}

// *************************** //
// SCHEDULAR FUNCTIONS
// *************************** //

int getRemainingTime(int thr_idx){
	return threads[thr_idx].count_max - threads[thr_idx].count; 
}

// lottery ticket schedular
void P_WF_scheduler(int signal) {
  int next_idx = -1;
  
  // calculate number of tickets.
  int num_tickets = getNumTickets();

  // get a random ticket
  int selected_ticket;
  if(num_tickets == 0)
    return; 
  selected_ticket = rand() % num_tickets;
  int cum_sum = 0;
  
  // determine the thread that has the selected ticket
	int i;
  for(i=1; i<ARRAY_SIZE; i++)
    if((threads[i].state == RUNNING) || (threads[i].state == READY)){
	    cum_sum += threads[i].tickets;
      if(selected_ticket < cum_sum){
        next_idx = i;
        int tmp = threads[i].tickets - 2;
        if(tmp < 0)
          threads[i].tickets = 0;
        else
          threads[i].tickets = tmp;
        break;
      }
    }

  // check if we found a valid idx
  if(next_idx != -1)
	  runThread(next_idx);
}

// shortest remaining time first algorithm
void SRTF_scheduler(int signal) {
	// get a random initial valid thread
	int shortest_idx = rand() % (ARRAY_SIZE - 1) + 1;
	while((threads[shortest_idx].state == FINISHED) || (threads[shortest_idx].state == EMPTY))
		shortest_idx = rand() % (ARRAY_SIZE - 1) + 1;
	
	// calculate the remaining time of each possible threads and choose the one that has the shortest remaining time
	int i;
	for(i=1; i<ARRAY_SIZE; i++)
		if((threads[i].state == RUNNING) || (threads[i].state == READY))
			if(getRemainingTime(i) < getRemainingTime(shortest_idx))
				shortest_idx = i;
			
	runThread(shortest_idx);
}

int main(int argc, char *argv[]) {
	int i;

	// Argument error check
	if(argc < 3){
		printf("Error: Too few arguments\n");
		printf("Usage: %s <schedular_function> <thread1_share> <thread2_share> .. <threadn_share>\n", argv[0]);
		printf("Example usage: %s p_wf 4 5 3 7 6 4\n", argv[0]);
		printf("Example usage: %s srtf 3 5 7\n", argv[0]);
		return 1;
	}
	if(strcmp(argv[1],"p_wf") == 0) {
		fun_sel = P_WF;
		signal( SIGALRM, P_WF_scheduler );
	}
	else if(strcmp(argv[1],"srtf") == 0) {
		fun_sel = SRTF;
		signal( SIGALRM, SRTF_scheduler );
	}
	else{
		printf("Error: Unknown schedular %s\n", argv[1]);
		printf("Usage: %s <schedular_function> <thread1_share> <thread2_share> .. <threadn_share>\n", argv[0]);
		printf("Example usage: %s p_wf 4 5 3 7 6 4\n", argv[0]);
		printf("Example usage: %s srtf 3 5 7\n", argv[0]);
		return 1;
	}
	// first, initialize all states to empty
  for(i=0; i<ARRAY_SIZE; i++)
    threads[i].state = EMPTY;

	// timer interrupt setting
	alarm(SWITCH_PERIODE_SEC);
	
	// Initialize main thread context
	initializeThread(0,0, -1);
	getcontext(&(threads[0].context));
	threads[0].state = READY;
	// Create threads
	int idx;
    for(i=2; i<argc; i++){
		while((idx = getEmptyIndex()) == -1) 
			raise(SIGALRM);		// no avaliable array position. wake up schedular
		initializeThread(idx, i-1, atoi(argv[i])); 
		createThread(idx, threadFun);
    }
	while(1)
		if(isAllFinished() == 1){	// if all threads are finished, print info and wait in an infinite loop
			// exit the main thread
			threads[0].state = FINISHED;
			free(threads[0].context.uc_stack.ss_sp);
			threads[0].context.uc_stack.ss_size = 0;
			if(finished_threads == NULL){
				finished_threads = (ThreadHistory*) malloc(sizeof(ThreadHistory));
				finished_threads->id = threads[0].id;
				finished_threads->next = NULL;
			}
			else{
				ThreadHistory *iter = finished_threads;
				while(iter->next != NULL)
					iter = iter->next;
				iter->next = (ThreadHistory*) malloc(sizeof(ThreadHistory));
				iter = iter->next;
				iter->id = threads[0].id;
				iter->next = NULL;
			}	
			printInfo();
			while(1);
		}
		else {// if all threads are not finished, wake up schedular
			raise(SIGALRM);
		}
   return 0;
}
