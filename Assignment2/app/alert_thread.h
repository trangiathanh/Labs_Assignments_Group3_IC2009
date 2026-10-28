#ifndef ALERT_THREAD_H
#define ALERT_THREAD_H

#include <stdio.h>
#include <stdlib.h>
#include "circular_buffer.h"

typedef struct {
    CircularBuffer *cb;
    volatile int *running;
} AlertArgs;

void* alert_thread_func(void *arg);

#endif /* ALERT_THREAD_H */
