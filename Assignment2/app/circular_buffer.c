#include <stdio.h>
#include "circular_buffer.h"

void cb_init(CircularBuffer *cb) {
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
    cb->total_produced = 0;
    cb->total_consumed = 0;
    cb->overflow_count = 0;
    pthread_mutex_init(&cb->mutex, NULL);
    pthread_cond_init(&cb->not_empty, NULL);
    pthread_cond_init(&cb->not_full, NULL);
}

void cb_destroy(CircularBuffer *cb) {
    pthread_mutex_destroy(&cb->mutex);
    pthread_cond_destroy(&cb->not_empty);
    pthread_cond_destroy(&cb->not_full);
}

int cb_push(CircularBuffer *cb, SensorData item) {
    pthread_mutex_lock(&cb->mutex);
    if (cb->count == BUFFER_CAPACITY) {
        cb->overflow_count++;
        pthread_mutex_unlock(&cb->mutex);
        return -1;
    }
    cb->data[cb->head] = item;
    cb->head = (cb->head + 1) % BUFFER_CAPACITY;
    cb->count++;
    cb->total_produced++;
    pthread_cond_signal(&cb->not_empty);
    pthread_mutex_unlock(&cb->mutex);
    return 0;
}

int cb_pop(CircularBuffer *cb, SensorData *item, volatile int *running) {
    pthread_mutex_lock(&cb->mutex);
    while (cb->count == 0 && *running) {
        pthread_cond_wait(&cb->not_empty, &cb->mutex);
    }
    if (cb->count == 0 && !(*running)) {
        pthread_mutex_unlock(&cb->mutex);
        return -1;
    }
    *item = cb->data[cb->tail];
    cb->tail = (cb->tail + 1) % BUFFER_CAPACITY;
    cb->count--;
    cb->total_consumed++;
    pthread_cond_signal(&cb->not_full);
    pthread_mutex_unlock(&cb->mutex);
    return 0;
}

size_t cb_get_produced(CircularBuffer *cb) { return cb->total_produced; }
size_t cb_get_consumed(CircularBuffer *cb) { return cb->total_consumed; }
size_t cb_get_size(CircularBuffer *cb) { return cb->count; }
size_t cb_get_overflow(CircularBuffer *cb) { return cb->overflow_count; }
