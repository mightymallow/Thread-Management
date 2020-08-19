// Statistics module
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int number_factories = 0;
typedef struct {
	double min_delay_ms;
	double max_delay_ms;
	double delay_sum_ms;
	int num_candies_rx;
	int num_candies_tx;
} stats_t;
static stats_t *stats = NULL;


static pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

void stats_init(int num_candy_factories)
{
	assert(num_candy_factories >= 0);
	number_factories = num_candy_factories;
	stats = malloc(sizeof(*stats) * num_candy_factories);
	memset(stats, 0, sizeof(*stats) * num_candy_factories);
}

void stats_cleanup(void)
{
	free(stats);
	stats = NULL;
}

void stats_record_produced(int factory_number)
{
	assert(factory_number >= 0);
	assert(factory_number < number_factories);

	pthread_mutex_lock(&stats_mutex);
	stats[factory_number].num_candies_tx += 1;
	pthread_mutex_unlock(&stats_mutex);

}

void stats_record_consumed(int factory_number, double delay_in_ms)
{
	assert(factory_number >= 0);
	assert(factory_number < number_factories);

	pthread_mutex_lock(&stats_mutex);
	stats_t *stats_rec = &stats[factory_number];
	if (stats_rec->num_candies_rx == 0) {
		stats_rec->min_delay_ms = delay_in_ms;
		stats_rec->max_delay_ms = delay_in_ms;
	} else {
		if (delay_in_ms > stats_rec->max_delay_ms) {
			stats_rec->max_delay_ms = delay_in_ms;
		}
		if (delay_in_ms < stats_rec->min_delay_ms) {
			stats_rec->min_delay_ms = delay_in_ms;
		}
	}
	stats_rec->delay_sum_ms += delay_in_ms;
	stats_rec->num_candies_rx += 1;
	pthread_mutex_unlock(&stats_mutex);
}

void stats_display(void)
{
	printf("Statistics:\n");
	printf("%10s%8s%8s%15s%15s%15s\n", "Factory#", "#Made", "#Eaten", "Min Delay[ms]", "Avg Delay[ms]", "Max Delay[ms]");
	for (int i = 0; i < number_factories; i++) {
		pthread_mutex_lock(&stats_mutex);
		double avg_delay = stats[i].delay_sum_ms / stats[i].num_candies_rx;
		printf("%10d%8d%8d%15.5f%15.5f%15.5f\n",
				i,
				stats[i].num_candies_tx,
				stats[i].num_candies_rx,
				stats[i].min_delay_ms,
				avg_delay,
				stats[i].max_delay_ms);
		if (stats[i].num_candies_rx != stats[i].num_candies_tx) {
			printf("ERROR! Candy creation/consumption mismatch.\n");
		}
		pthread_mutex_unlock(&stats_mutex);
	}
}
