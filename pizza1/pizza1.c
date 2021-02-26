#include "pizza1.h"

//Mutexes

pthread_mutex_t mutex_number_available_cooks = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_number_available_ovens = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_console = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_total_waiting_time = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_max_waiting_time = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_rand_r = PTHREAD_MUTEX_INITIALIZER;

//Conditions

pthread_cond_t cond_number_available_cooks = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_number_available_ovens = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_available_console = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_available_t_w_t = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_available_m_w_t = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_rand_r = PTHREAD_COND_INITIALIZER;

//Variables

int number_available_cooks;
int number_available_ovens;
double total_waiting_time;
double max_waiting_time;
unsigned int seed;


// Handling of mutexe's lock/unlock errors. Response code is 0 when mutex works properly
void successful_mutex_action(int response_code)
{
	if (response_code != 0)
	{
		printf("Error in mutex lock/unlock! Error Code: %d \n", response_code);

	pthread_exit(&response_code);
	}
}

// Preparation and baking for each order
void * order(void * order_id)
{	
	double waiting_time = 0;
	int response_code;
	int oid = *(int *)order_id;
	struct timespec order_start;
	struct timespec order_finish;
	
	clock_gettime(CLOCK_REALTIME, &order_start); //start time counting

	
	response_code = pthread_mutex_lock(&mutex_number_available_cooks);
	successful_mutex_action(response_code);

	while(number_available_cooks <= 0)	//checks the availability of cooks
	{
		printf("Order %d waiting for an available cook...\n", oid);	//waiting
		pthread_cond_wait(&cond_number_available_cooks, &mutex_number_available_cooks);
	}
	
	number_available_cooks--;
	printf("Under preparation, order %d\n", oid); // starts the preparation of order

	pthread_cond_signal(&cond_number_available_cooks);
	response_code = pthread_mutex_unlock(&mutex_number_available_cooks);
	successful_mutex_action(response_code);
	
	
	response_code = pthread_mutex_lock(&mutex_rand_r);
	successful_mutex_action(response_code);
	
	int random_number_pizza = rand_r(&seed) % Norderhigh + Norderlow; // the random number for order's pizzas
	
	pthread_cond_signal(&cond_rand_r);
	response_code = pthread_mutex_unlock(&mutex_rand_r);
	successful_mutex_action(response_code);
	
	int prep_time = Tprep*random_number_pizza;
	sleep(prep_time); //order preparation time
	
	response_code = pthread_mutex_lock(&mutex_number_available_ovens);
	successful_mutex_action(response_code);

	while(number_available_ovens <= 0)	//checks the availability of ovens
	{	
		printf("Order %d waiting for an available oven...\n", oid);	//waiting...
		pthread_cond_wait(&cond_number_available_ovens, &mutex_number_available_ovens);
	}

	number_available_ovens--;
	printf("Baking, order %d\n", oid);//starts baking operation

	pthread_cond_signal(&cond_number_available_ovens);	
	response_code = pthread_mutex_unlock(&mutex_number_available_ovens);
	successful_mutex_action(response_code);

	sleep(Tbake);//baking time

	response_code = pthread_mutex_lock(&mutex_number_available_ovens);
	successful_mutex_action(response_code);

	number_available_ovens++; // oven release

	pthread_cond_signal(&cond_number_available_ovens);	
	response_code = pthread_mutex_unlock(&mutex_number_available_ovens);
	successful_mutex_action(response_code);

	response_code = pthread_mutex_lock(&mutex_number_available_cooks);
	successful_mutex_action(response_code);
	
	number_available_cooks++; // cook release

	pthread_cond_signal(&cond_number_available_cooks);
	response_code = pthread_mutex_unlock(&mutex_number_available_cooks);
	successful_mutex_action(response_code);

	clock_gettime(CLOCK_REALTIME, &order_finish); // finish time

	//print order info

	response_code = pthread_mutex_lock(&mutex_console);
	successful_mutex_action(response_code);

	waiting_time = order_finish.tv_sec - order_start.tv_sec;
	printf("Order %d completed in %0.2f minutes. Output %d pizzas \n", oid, waiting_time, random_number_pizza);
	
	pthread_cond_signal(&cond_available_console);	
	response_code = pthread_mutex_unlock(&mutex_console);
	successful_mutex_action(response_code);

	//update total_waiting_time
	response_code = pthread_mutex_lock(&mutex_total_waiting_time);
	successful_mutex_action(response_code);

	total_waiting_time += waiting_time;

	pthread_cond_signal(&cond_available_t_w_t);	
	response_code = pthread_mutex_unlock(&mutex_total_waiting_time);
	successful_mutex_action(response_code);
	

	//update max_waiting_time if current waiting time is bigger than the current max waiting time
	response_code = pthread_mutex_lock(&mutex_max_waiting_time);
	successful_mutex_action(response_code);

	if (waiting_time > max_waiting_time)
	{
		max_waiting_time = waiting_time;
	}

	pthread_cond_signal(&cond_available_m_w_t);	
	response_code = pthread_mutex_unlock(&mutex_max_waiting_time);
	successful_mutex_action(response_code);
	
	pthread_exit(NULL); // exit thread execution
}


int main(int argc, char *argv[])
{
	if (argc != 3) // argc[0] = ./a.out --> argc[1] = number of customers --> argc[2] = seed 
	{
		printf("ERROR: Program needs exactly 2 arguments (number of customers --> int, seed --> int)\n\n\n");
		exit(-1); //exit pogramm with code -1
	}
	
	int number_customers = atoi(argv[1]);
	if (number_customers <= 0) //we need at least one customer to place an order
	{
		printf("ERROR: Customers number must be integer bigger than zero \n");
		exit(-1); //exit pogramm with code -1
	}

	waiting_time = 0;
	total_waiting_time = 0;
	max_waiting_time = 0;
	number_available_cooks = Ncook;
	number_available_ovens = Noven;
	seed = atoi(argv[2]);
	
	printf("\n\n Welcome to Nick's Pizza! \nYou have chosen %d customers and seed %d\n\n\n", number_customers, seed);
	

	//Initializing orders id

	int order_id[number_customers];

	for(int i=0; i<number_customers; i++)
	{
		order_id[i] = i+1; // IDs counting from 1
	}

	//Creating threads id

	pthread_t thread_id[number_customers];
	
	for(int i=0; i<number_customers; i++)
	{	
		printf("Order %d just placed\n", i+1);
		pthread_create(&thread_id[i], NULL, &order, &order_id[i]);
		
		response_code = pthread_mutex_lock(&mutex_rand_r);
		successful_mutex_action(response_code);
		
		double random_order_time = rand_r(&seed) % Torderhigh + Torderlow; //new order after random time
		
		pthread_cond_signal(&cond_rand_r);
		response_code = pthread_mutex_unlock(&mutex_rand_r);
		successful_mutex_action(response_code);
		
		sleep(random_order_time); // waiting for new order
	}

	//Wait all threads to finish

	for(int i=0; i<number_customers; i++)
	{
		pthread_join(tid[i], NULL);
	}

	//Print results

	printf("Average waiting time was: %0.2f \n", total_waiting_time/number_customers);
	printf("Max waiting time was: %0.2f \n", max_waiting_time);
	
	
	//Destroy mutexes
	pthread_mutex_destroy(&mutex_number_available_cooks);
	pthread_mutex_destroy(&mutex_number_available_ovens);
	pthread_mutex_destroy(&mutex_console);
	pthread_mutex_destroy(&mutex_total_waiting_time);
	pthread_mutex_destroy(&mutex_max_waiting_time);
	pthread_mutex_destroy(&mutex_rand_r);

	//Destroy conditions
	pthread_cond_destroy(&cond_number_available_cooks);
	pthread_cond_destroy(&cond_number_available_ovens);
	pthread_cond_destroy(&cond_available_console);
	pthread_cond_destroy(&cond_available_t_w_t);
	pthread_cond_destroy(&cond_available_m_w_t);
	pthread_cond_destroy(&cond_rand_r);

	return 0;
}


	

	

	


	


