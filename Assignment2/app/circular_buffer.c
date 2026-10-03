#include <stdlib.h>
#include "circular_buffer.h"

void cb_init(CircularBuffer *cb) {
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
    pthread_mutex_init(&cb->mutex, NULL);
    pthread_cond_init(&cb->not_empty, NULL);
    pthread_cond_init(&cb->not_full, NULL);
}

void cb_destroy(CircularBuffer *cb) {
    pthread_mutex_destroy(&cb->mutex);
    pthread_cond_destroy(&cb->not_empty);
    pthread_cond_destroy(&cb->not_full);
}

int cb_push(CircularBuffer *cb, SensorData data) {
    pthread_mutex_lock(&cb->mutex);
    while (cb->count == BUFFER_SIZE) {
        pthread_cond_wait(&cb->not_full, &cb->mutex);
    }
    cb->buffer[cb->head] = data;
    cb->head = (cb->head + 1) % BUFFER_SIZE;
    cb->count++;
    pthread_cond_signal(&cb->not_empty);
    pthread_mutex_unlock(&cb->mutex);
    return 0;
}

int cb_pop(CircularBuffer *cb, SensorData *data) {
    pthread_mutex_lock(&cb->mutex);
    while (cb->count == 0) {
        pthread_cond_wait(&cb->not_empty, &cb->mutex);
    }
    *data = cb->buffer[cb->tail];
    cb->tail = (cb->tail + 1) % BUFFER_SIZE;
    cb->count--;
    pthread_cond_signal(&cb->not_full);
    pthread_mutex_unlock(&cb->mutex);
    return 0;
}
