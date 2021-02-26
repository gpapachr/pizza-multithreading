#include "p3180141-p3180150-p3180161-pizza2.h"

//Mutexes

pthread_mutex_t mutex_number_available_cooks      = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_number_available_ovens      = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_number_available_deliverers = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_console                     = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_total_waiting_time          = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_max_waiting_time            = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_total_cooling_time          = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_max_cooling_time            = PTHREAD_MUTEX_INITIALIZER;

//Conditions

pthread_cond_t cond_number_available_cooks       = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_number_available_ovens       = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_number_available_deliverers  = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_available_console            = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_available_t_w_t              = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_available_m_w_t              = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_available_t_c_t              = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_available_m_c_t              = PTHREAD_COND_INITIALIZER;

//Variables

int number_available_cooks;
int number_available_ovens;
int number_available_deliverers;
double waiting_time;
double cooling_time;
double total_waiting_time;
double total_cooling_time;
double max_waiting_time;
double max_cooling_time;

unsigned int seed;


// Handling of mutexe's lock/unlock errors
void successful_mutex_action(int response_code)
{
	if (response_code != 0)
	{
		printf("Error in mutex lock/unlock! Error Code: %d \n", response_code);

	pthread_exit(&response_code);
	}
}


void * order(void * order_id)
{	
	waiting_time = 0;
	int response_code;
	int oid = *(int *)order_id;
	struct timespec order_start;
	struct timespec order_finish;
	struct timespec cooling_start;
	struct timespec cooling_finish;
	
	clock_gettime(CLOCK_REALTIME, &order_start); //starting time

	
	response_code = pthread_mutex_lock(&mutex_number_available_cooks);
	
	successful_mutex_action(response_code);

	while(number_available_cooks <= 0)
	{
		printf("Order %d waiting for an available cook...\n", oid);
		pthread_cond_wait(&cond_number_available_cooks, &mutex_number_available_cooks);
	}
	
	number_available_cooks--;
	
	printf("Under preparation, order %d\n", oid);

	response_code = pthread_cond_signal(&cond_number_available_cooks);
	response_code = pthread_mutex_unlock(&mutex_number_available_cooks);
	successful_mutex_action(response_code);

	

	int random_number_pizza = rand_r(&seed) % Norderhigh + Norderlow;
	int prep_time = Tprep*random_number_pizza;
	sleep(prep_time); //order preparation
	
	response_code = pthread_mutex_lock(&mutex_number_available_ovens);
	successful_mutex_action(response_code);

	

	while(number_available_ovens <= 0)
	{	
		printf("Order %d waiting for an available oven...\n", oid);
		pthread_cond_wait(&cond_number_available_ovens, 		     &mutex_number_available_ovens);
	}

	number_available_ovens--;
	
	printf("Baking, order %d\n", oid);

	response_code = pthread_mutex_lock(&mutex_number_available_cooks);
	successful_mutex_action(response_code);

	
	
	number_available_cooks++; // cook release
	

	pthread_cond_signal(&cond_number_available_cooks);
	response_code = pthread_mutex_unlock(&mutex_number_available_cooks);
	successful_mutex_action(response_code);

	

	pthread_cond_signal(&cond_number_available_ovens);	
	response_code = pthread_mutex_unlock(&mutex_number_available_ovens);
	successful_mutex_action(response_code);

	

	sleep(Tbake);//baking

	clock_gettime(CLOCK_REALTIME, &cooling_start); //starting cooling time

	response_code = pthread_mutex_lock(&mutex_number_available_deliverers);
	successful_mutex_action(response_code);

	

	while(number_available_deliverers <= 0)
	{	
		printf("Order %d waiting for an available deliverer...\n", oid);
		pthread_cond_wait(&cond_number_available_deliverers, 		     &mutex_number_available_deliverers);
	}
	

	number_available_deliverers--;
	printf("Delivering, order %d\n", oid);
	

	response_code = pthread_mutex_lock(&mutex_number_available_ovens);
	successful_mutex_action(response_code);

	
	number_available_ovens++; // oven release
	
	
	
	pthread_cond_signal(&cond_number_available_ovens);	
	response_code = pthread_mutex_unlock(&mutex_number_available_ovens);
	successful_mutex_action(response_code);

	pthread_cond_signal(&cond_number_available_deliverers);	
	response_code = pthread_mutex_unlock(&mutex_number_available_deliverers);
	successful_mutex_action(response_code);

	double Tdelivery = rand_r(&seed) % Thigh + Tlow;
	
	sleep(Tdelivery); // Delivering pizza

	clock_gettime(CLOCK_REALTIME, &cooling_finish); //finish cooling time

	sleep(Tdelivery); // Returning to pizzeria

	
	response_code = pthread_mutex_lock(&mutex_number_available_deliverers);
	successful_mutex_action(response_code);
	
	number_available_deliverers++; // deliverer release

	pthread_cond_signal(&cond_number_available_deliverers);
	response_code = pthread_mutex_unlock(&mutex_number_available_deliverers);
	successful_mutex_action(response_code);

	clock_gettime(CLOCK_REALTIME, &order_finish); // finish time

//print order info

	response_code = pthread_mutex_lock(&mutex_console);
	successful_mutex_action(response_code);

	waiting_time = cooling_finish.tv_sec - order_start.tv_sec;
	cooling_time = cooling_finish.tv_sec - cooling_start.tv_sec;

	printf("Order %d (%d pizzas) delivered in %0.2f minutes with %0.2f minutes of cooling.\n", oid, random_number_pizza, waiting_time, cooling_time);
	
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

//update total_cooling_time
	response_code = pthread_mutex_lock(&mutex_total_cooling_time);
	successful_mutex_action(response_code);

	total_cooling_time += cooling_time;

	pthread_cond_signal(&cond_available_t_c_t);	
	response_code = pthread_mutex_unlock(&mutex_total_cooling_time);
	successful_mutex_action(response_code);
	

//update max_waiting_time
	response_code = pthread_mutex_lock(&mutex_max_waiting_time);
	successful_mutex_action(response_code);

	if (waiting_time > max_waiting_time)
	{
		max_waiting_time = waiting_time;
	}

	pthread_cond_signal(&cond_available_m_w_t);	
	response_code = pthread_mutex_unlock(&mutex_max_waiting_time);
	successful_mutex_action(response_code);
	
	

//update max_cooling_time
	response_code = pthread_mutex_lock(&mutex_max_cooling_time);
	successful_mutex_action(response_code);

	if (cooling_time > max_cooling_time)
	{
		max_cooling_time = cooling_time;
	}

	pthread_cond_signal(&cond_available_m_c_t);	
	response_code = pthread_mutex_unlock(&mutex_max_cooling_time);
	successful_mutex_action(response_code);
	
	pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("ERROR: Program needs exactly 2 arguments (number of customers --> int, seed --> int)\n\n\n");
		exit(-1);
	}
	
	int number_customers = atoi(argv[1]);
	if (number_customers <= 0)
	{
		printf("ERROR: Customers number must be integer bigger than zero \n");
		exit(-1);
	}

	waiting_time = 0;
	cooling_time = 0;
	total_waiting_time = 0;
	total_cooling_time = 0;
	max_waiting_time = 0;
	max_cooling_time = 0;
	number_available_cooks = Ncook;
	number_available_ovens = Noven;
	number_available_deliverers = Ndeliverer;
	seed = atoi(argv[2]);
	
	printf("\n\n Welcome to Nick's Pizza!\nYou have chosen %d customers and seed %d\n\n\n", number_customers, seed);
	

	//Initializing orders id

	int order_id[number_customers];

	for(int i=0; i<number_customers; i++)
	{
		order_id[i] = i+1;
	}

	//Creating threads id

	pthread_t tid[number_customers];
	
	for(int i=0; i<number_customers; i++)
	{	
		printf("Order %d just placed\n", i+1);
		pthread_create(&tid[i], NULL, &order, &order_id[i]);
		double random_order_time = rand_r(&seed) % Torderhigh + Torderlow;
		sleep(random_order_time); // waiting for new order
	}

	//Wait all threads to finish

	for(int i=0; i<number_customers; i++)
	{
		pthread_join(tid[i], NULL);
	}

	//Print results
	
	printf("\n----------Operation Summary----------\n");
	printf("Average waiting time was : %0.2f \n", total_waiting_time/number_customers);
	printf("Max waiting time was     : %0.2f \n", max_waiting_time);
	printf("Average cooling time was : %0.2f \n", total_cooling_time/number_customers);
	printf("Max cooling time was     : %0.2f \n\n", max_cooling_time);

	pthread_mutex_destroy(&mutex_number_available_cooks);
	pthread_mutex_destroy(&mutex_number_available_ovens);
	pthread_mutex_destroy(&mutex_number_available_deliverers);
	pthread_mutex_destroy(&mutex_console);
	pthread_mutex_destroy(&mutex_total_waiting_time);
	pthread_mutex_destroy(&mutex_max_waiting_time);
	pthread_mutex_destroy(&mutex_total_cooling_time);
	pthread_mutex_destroy(&mutex_max_cooling_time);


	pthread_cond_destroy(&cond_number_available_cooks);
	pthread_cond_destroy(&cond_number_available_ovens);
	pthread_cond_destroy(&cond_number_available_deliverers);
	pthread_cond_destroy(&cond_available_console);
	pthread_cond_destroy(&cond_available_t_w_t);
	pthread_cond_destroy(&cond_available_m_w_t);
	pthread_cond_destroy(&cond_available_t_c_t);
	pthread_cond_destroy(&cond_available_m_c_t);


	return 0;
}

		



		

	

	

	


	


