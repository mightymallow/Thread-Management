// Thread-safe Bounded Buffer queue
// Client code responsible for allocating and freeing items being
// stored in the buffer.
#ifndef BBUFF_H
#define BBUFF_H

#define QUEUE_SIZE 10

void bbuff_init(void);
void bbuff_blocking_insert(void* item);
void* bbuff_blocking_extract(void);
_Bool bbuff_is_data_available(void);

#endif
