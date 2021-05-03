#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#define false 0
#define true 1
typedef int bool;

//*******************************************************************//
/************* DATA STRUCTURES **************/
//*******************************************************************//
// bank client data structure
typedef struct BankClient{
       	int id; // ClientID
       	double duration; // banking transactionduration
} BankClient;

// information data structure
typedef struct Information {
       	int paydeskNo;
       	struct BankClient client;
} Information;

// client queue
typedef struct ClientQueue {
       	BankClient* array;
       	int maxSize;
       	int currentSize;
       	int frontIndex;
	int rearIndex;
} ClientQueue; 

// Bank desk
typedef struct PayDesk {
	int desk_no;
	ClientQueue *client_queue;
	pthread_mutex_t lock;
	pthread_cond_t cond;
} PayDesk;

//*******************************************************************//
/************** GLOBAL VARIABLES **************/
//*******************************************************************//
int NUM_CLIENTS = 20;
int NUM_DESKS = 4;
int QUEUE_SIZE = 3;
float DURATION_RATE = 10.0;
float GENERATION_RATE = 100.0;
Information *info;
bool print_info;
bool END_OF_CLIENTS;
bool TERMINATE;
pthread_mutex_t end_of_clients_lock;
pthread_mutex_t info_lock;
pthread_cond_t info_cond;
PayDesk *PAY_DESKS;

//*******************************************************************//
/************** USEFUL FUNCTIONS **************/
//*******************************************************************//
// initialize queue
void initQueue(ClientQueue *client_queue, int maxSize) {
	BankClient *array = (BankClient*)malloc(sizeof(BankClient)*maxSize);
	client_queue->array = array;
	client_queue->maxSize = maxSize;
    client_queue->currentSize = 0;
	client_queue->frontIndex = 0;
	client_queue->rearIndex = 0;
}

// initialize pay desk
void initPayDesk(PayDesk *pay_desk, int desk_no, int maxSize, int n){
	pthread_mutex_init(&pay_desk->lock, NULL); 
	pthread_cond_init(&pay_desk->cond, NULL);
	pay_desk->desk_no = desk_no;
	pay_desk->client_queue = (ClientQueue*) malloc(sizeof(ClientQueue)*n);
	initQueue(pay_desk->client_queue, maxSize);
}

// initialize 'Information'. Initializes the global 'Information'.
void initInfo(){
	pthread_mutex_init(&info_lock, NULL); 
	pthread_cond_init(&info_cond, NULL);
	info = (Information*) malloc(sizeof(Information));
	print_info = false;
}

// insert function for queue
void insertQueue(int idx, BankClient client){
	ClientQueue *client_queue = PAY_DESKS[idx].client_queue;
	if(client_queue->currentSize >= client_queue->maxSize){
		printf("Cannot insert. Queue is full!\n");
		return;
	}

	int rear = client_queue->rearIndex;
	client_queue->array[rear++] = client;
	client_queue->rearIndex = rear % (client_queue->maxSize);
	client_queue->currentSize++;
	//printf("Rear: %d; CurrSize: %d; maxSize: %d\n",client_queue->rearIndex, client_queue->currentSize, client_queue->maxSize);
}

// delete function for queue
BankClient deleteQueue(int idx){
	ClientQueue * client_queue = PAY_DESKS[idx].client_queue;
	int front = client_queue->frontIndex;
	BankClient bank_client;
	if(client_queue->currentSize > 0) {
		bank_client = client_queue->array[front++];
		client_queue->frontIndex = front % client_queue->maxSize;
		//printf("BEFORE: %d\n", client_queue->currentSize);
		client_queue->currentSize--;
		//printf("AFTER: %d\n", client_queue->currentSize);
	}
	else
		printf("Cannot pop. Queue is empty!\n");
	return bank_client;	
}

void insertClient(int num_desk, int max_size, BankClient client){
	// find the desk with smallest number of clients
	int i, j, smallest_size=max_size+1, smallest_idx=-1;
	// start to look for smallest queue size from a random pay desk
	j = rand() % num_desk;
	for(i=0; i<num_desk; i++){
		j = (j + i) % num_desk;
		// enter the critical section
		pthread_mutex_lock(&(PAY_DESKS[j].lock));
		ClientQueue *queue = PAY_DESKS[j].client_queue;
		if(queue->currentSize < smallest_size){
			smallest_size = queue->currentSize;
			smallest_idx = j;
		}
		// exit the critical section
		pthread_mutex_unlock(&(PAY_DESKS[j].lock));
	}
	
	// desks are full case
	if(smallest_idx == -1)
		return;
	
	// insert the client to the found desk
	pthread_mutex_lock(&(PAY_DESKS[smallest_idx].lock));
	insertQueue(smallest_idx, client);
	//if((PAY_DESKS[smallest_idx].client_queue)->currentSize == 1)
	pthread_cond_signal(&(PAY_DESKS[smallest_idx].cond));
	pthread_mutex_unlock(&(PAY_DESKS[smallest_idx].lock));
}

//*******************************************************************//
//************** THREAD FUNCTIONS **************//
//*******************************************************************//
// pay desk thread
void* payDesk(void *argdesk){
	// argument is the desk index
	int i = *(int*)argdesk;
	while(1){		
		// start critical section
		pthread_mutex_lock(&PAY_DESKS[i].lock);
		
		while((PAY_DESKS[i].client_queue)->currentSize <= 0){
			if(END_OF_CLIENTS)
				pthread_exit(NULL);
			pthread_cond_wait(&(PAY_DESKS[i].cond), &(PAY_DESKS[i].lock));
		}
		
		
		// We have the lock and there is at least one client
		// Handle the client at the front
		BankClient bank_client = deleteQueue(i);
		pthread_mutex_unlock(&PAY_DESKS[i].lock);
		
		sleep(bank_client.duration);	
		
		// update info
		pthread_mutex_lock(&info_lock);
		info->paydeskNo = i;
		info->client = bank_client;
		print_info = true;

		// there is an info to be printed. wakeup info thread
		pthread_cond_signal(&info_cond);

		pthread_mutex_unlock(&info_lock);
	}
	
}

// Information print thread.
void* infoThread(void* arginfo){
	while(1){	
		// enter critical setcion
		pthread_mutex_lock(&info_lock);

		while(!print_info){
			if(TERMINATE)
				pthread_exit(NULL);
			pthread_cond_wait(&info_cond, &info_lock);
		}

		printf("Desk %d served Client %d in %f seconds.\n", info->paydeskNo, (info->client).id, (info->client).duration);
		print_info = false;

		// exit critical section
		pthread_mutex_unlock(&info_lock);	
	}
}

//*******************************************************************//
/************** MAIN THREAD **************/
//*******************************************************************//
int main(int argc, char *argv[]){
	// parse the arguments
	// srand(80233423);
	int option;
	while((option = getopt(argc, argv, ":c:n:q:g:d:")) !=-1)
	switch(option){
		case 'c': NUM_CLIENTS = atoi(optarg); break;
		case 'n': NUM_DESKS = atoi(optarg); break;
		case 'q': QUEUE_SIZE = atoi(optarg); break;
		case 'g': GENERATION_RATE = atof(optarg); break;
		case 'd': DURATION_RATE = atof(optarg); break;
	}
	
	// print parsed arguments
	printf("NUM_CLIENTS      : %d\n", NUM_CLIENTS);
	printf("NUM_DESKS        : %d\n", NUM_DESKS);
	printf("QUEUE_SIZE       : %d\n", QUEUE_SIZE);
	printf("DURATION_RATE    : %f\n", DURATION_RATE);
	printf("GENERATION_RATE  : %f\n", GENERATION_RATE);
	
	// initialize END_OF_CLIENTS
	pthread_mutex_init(&end_of_clients_lock, NULL);
	END_OF_CLIENTS = false;
	TERMINATE = false;
	// initialize Information struct parameters
	initInfo();
	// create information printer thread
	pthread_t info_thread;
	pthread_create(&info_thread, NULL, infoThread, NULL);
	//pthread_join(info_thread, NULL);
	// create desks, desk threads and associated client queues
	int i;
	PAY_DESKS = (PayDesk*) malloc(sizeof(PayDesk)*NUM_DESKS);
	pthread_t desk_threads[NUM_DESKS];
	int idxs[NUM_DESKS];
	for(i=0; i<NUM_DESKS; i++){
		initPayDesk(&PAY_DESKS[i], i, QUEUE_SIZE, NUM_DESKS);
		idxs[i] = i;
		// create pay desk threads
		pthread_create(&desk_threads[i], NULL, payDesk, &idxs[i]);
	}

	// generate 'c' indivudial clients
	int next_id = 0;
	for(i=0; i<NUM_CLIENTS; i++){
		BankClient bank_client;
		// assign an id to the client
		bank_client.id = next_id++;
	
		// generate a random duration time for the client
		double x = ((double) rand() / (RAND_MAX));
		bank_client.duration = -log(1-x) / DURATION_RATE;
	
		// inform that a client is arrived
		printf("Client %d arrived.\n", bank_client.id);
		
		// generate random sleep time
		x = ((double) rand() / (RAND_MAX));
		double sleep_duration = -log(1-x) / GENERATION_RATE;
		
		// insert the client to the queue with smallest size
		insertClient(NUM_DESKS, QUEUE_SIZE, bank_client);
		// sleep
		sleep(sleep_duration);
	}
	// end of clients
	END_OF_CLIENTS = true;
	sleep(0.5);
	for(i=0; i<NUM_DESKS; i++){
		// wake up desk threads and wait for them to terminate
		pthread_cond_signal(&(PAY_DESKS[i].cond));
		pthread_join(desk_threads[i], NULL);
	}

	TERMINATE = true;
	sleep(0.5);
	// wake up information print thread and wait for it to terminate
	pthread_cond_signal(&info_cond);
	pthread_join(info_thread, NULL);
	return 0;
}