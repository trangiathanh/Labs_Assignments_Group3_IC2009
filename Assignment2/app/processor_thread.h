#ifndef PROCESSOR_THREAD_H
#define PROCESSOR_THREAD_H

#include <stdio.h>
#include <stdlib.h>
#include "circular_buffer.h"

typedef struct {
    CircularBuffer *cb;
    volatile int *running;
} ProcessorArgs;

void* processor_thread_func(void *arg);

#endif /* PROCESSOR_THREAD_H */
