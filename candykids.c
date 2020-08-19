// Solution to the candy-kids (producer-consumer) problem
// By Brian Fraser

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include "bbuff.h"
#include "stats.h"


/**
 * Constants
 */
#define MAX_FACTORY_WAIT 3
#define MAX_KID_WAIT 1

#define ARGS_PROGRAM         0
#define ARGS_FACTORY_THREADS 1
#define ARGS_KID_THREADS     2
#define ARGS_SECONDS         3
#define ARG_COUNT            (ARGS_SECONDS + 1)

/**
 * Candy data to produce/consume
 */
typedef struct  {
	int source_thread;
	double time_stamp_in_ms;
} candy_t;


/*
 * Helper Fuctions
 */
double current_time_in_ms(void)
{
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	return now.tv_sec * 1000.0 + now.tv_nsec/1000000.0;
}

/**
 * Threads
 */
_Bool stop_factory_threads = false;

void* factory_thread(void* arg)
{
	int factory_num = *(int *) arg;

	while (!stop_factory_threads) {

		int wait_s = rand() % (MAX_FACTORY_WAIT + 1);
		printf("\tFactory %d ship candy & wait %ds\n", factory_num, wait_s);

		// Create work item
		candy_t *item = malloc(sizeof(*item));
		item->source_thread = factory_num;
		item->time_stamp_in_ms = current_time_in_ms();
		stats_record_produced(factory_num);
		bbuff_blocking_insert(item);

		// Delay
		sleep(wait_s);
	}

	printf("Candy-factory %d done\n", factory_num);
	pthread_exit(0);
}

void* kid_thread(void* arg)
{
	while (true) {
		// Thread may be canceled while blocking here
		candy_t *item = bbuff_blocking_extract();

		double delay_in_ms = current_time_in_ms() - item->time_stamp_in_ms;

		stats_record_consumed(item->source_thread, delay_in_ms);
		free(item);

		int wait_s = rand() % (MAX_KID_WAIT + 1);
		sleep(wait_s);
	}
	pthread_exit(0);
}

/*
 * Main
 */
int main(int argc, char **argv)
{

	if (argc < ARG_COUNT) {
		printf("Usage: %s #factories #kids #seconds-to-run\n", argv[ARGS_PROGRAM]);
		exit(1);
	}
	int num_factories = atoi(argv[ARGS_FACTORY_THREADS]);
	int num_kids = atoi(argv[ARGS_KID_THREADS]);
	int num_seconds = atoi(argv[ARGS_SECONDS]);

	if (num_factories <= 0 || num_kids <= 0 || num_seconds <= 0) {
		printf("Error: All arguments must be positive.\n");
		exit(1);
	}

	// Initialize modules
	srand(time(0));
	bbuff_init();
	stats_init(num_factories);

	// Launch thread
	pthread_t tids_factories[num_factories];
	int thread_args_factories[num_factories];
	for (int i = 0; i < num_factories; i++) {
		thread_args_factories[i] = i;
		pthread_create(&tids_factories[i], NULL, factory_thread, &thread_args_factories[i]);
	}

	pthread_t tids_kids[num_kids];
	for (int i = 0; i < num_kids; i++) {
		pthread_create(&tids_kids[i], NULL, kid_thread, NULL);
	}


	// Wait for time:
	for (int i = 0; i < num_seconds; i++) {
		printf("Time %2ds:\n", i);
		sleep(1);
	}

	printf("Stopping candy factories...\n");
	stop_factory_threads = true;
	// Wait until threads are done
	for (int i = 0; i < num_factories; i++) {
		pthread_join(tids_factories[i], NULL);
	}

	while (bbuff_is_data_available()) {
		printf("Waiting for all candy to be consumed.\n");
		sleep(1);
	}
	printf("Stopping kids.\n");
	for (int i = 0; i < num_kids; i++) {
		pthread_cancel(tids_kids[i]);
		pthread_join(tids_kids[i], NULL);
	}

	stats_display();
	stats_cleanup();
}
