// Bounded buffer implementation using semaphores
#include "bbuff.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>


static void* queue[QUEUE_SIZE];
static int queue_write_idx = 0;
static int queue_read_idx = 0;

// Semaphores:
static sem_t haveData;
static sem_t haveSpace;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void bbuff_init(void)
{
	if (sem_init(&haveData, 0, 0) != 0) {
		perror("Semaphore initialization failed.\n");
		exit(-1);
	}
	if (sem_init(&haveSpace, 0, QUEUE_SIZE) != 0) {
		perror("Semaphore initialization failed.\n");
		exit(EXIT_FAILURE);
	}
}

void bbuff_blocking_insert(void* item)
{
	sem_wait(&haveSpace);
	pthread_mutex_lock(&mutex);

	queue[queue_write_idx] = item;
	queue_write_idx = (queue_write_idx + 1) % QUEUE_SIZE;

	pthread_mutex_unlock(&mutex);
	sem_post(&haveData);
}

void* bbuff_blocking_extract(void)
{
	sem_wait(&haveData);
	pthread_mutex_lock(&mutex);

	void* item = queue[queue_read_idx];
	queue[queue_read_idx] = NULL;
	queue_read_idx = (queue_read_idx + 1) % QUEUE_SIZE;

	pthread_mutex_unlock(&mutex);
	sem_post(&haveSpace);

	return item;
}

_Bool bbuff_is_data_available(void)
{
	int count = 0;
	if (sem_getvalue(&haveData, &count) != 0) {
		perror("Unable to read semaphore value.");
		exit(EXIT_FAILURE);
	}
	return count > 0;
}
