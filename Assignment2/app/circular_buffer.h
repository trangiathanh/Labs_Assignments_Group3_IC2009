#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <pthread.h>
#include "../driver/sms_sensor.h"

#define BUFFER_SIZE 64

typedef struct {
    SensorData buffer[BUFFER_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} CircularBuffer;

void cb_init(CircularBuffer *cb);
void cb_destroy(CircularBuffer *cb);
int cb_push(CircularBuffer *cb, SensorData data);
int cb_pop(CircularBuffer *cb, SensorData *data);

#endif
