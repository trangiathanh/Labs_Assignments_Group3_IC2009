#ifndef SENSOR_THREAD_H
#define SENSOR_THREAD_H

#include "circular_buffer.h"

typedef struct {
    CircularBuffer *cb;
    volatile int *running;
} SensorArgs;

void* sensor_thread_func(void *arg);

#endif /* SENSOR_THREAD_H */
