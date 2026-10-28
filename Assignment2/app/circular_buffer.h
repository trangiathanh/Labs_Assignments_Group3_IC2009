#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <pthread.h>
#include <stddef.h>

#define BUFFER_CAPACITY 16

typedef struct {
    unsigned long timestamp;
    float temperature;
    float humidity;
} SensorData;

typedef struct {
    SensorData data[BUFFER_CAPACITY];
    int head;
    int tail;
    size_t count;
    size_t total_produced;
    size_t total_consumed;
    size_t overflow_count;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} CircularBuffer;

void cb_init(CircularBuffer *cb);
void cb_destroy(CircularBuffer *cb);
int cb_push(CircularBuffer *cb, SensorData item);
int cb_pop(CircularBuffer *cb, SensorData *item, volatile int *running);

size_t cb_get_produced(CircularBuffer *cb);
size_t cb_get_consumed(CircularBuffer *cb);
size_t cb_get_size(CircularBuffer *cb);
size_t cb_get_overflow(CircularBuffer *cb);

#endif /* CIRCULAR_BUFFER_H */
