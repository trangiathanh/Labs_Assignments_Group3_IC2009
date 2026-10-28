#include <pthread.h>
#include <stdio.h>

#define BUFFER_SIZE 10

typedef struct {
    long long timestamp_ms;
    float temperature;
    float humidity;
} SensorData;

SensorData circular_buffer[BUFFER_SIZE];
int buffer_head = 0;
int buffer_tail = 0;
int buffer_count = 0;

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_not_empty = PTHREAD_COND_INITIALIZER;

/* --- ĐỒNG BỘ LUỒNG: GHI DỮ LIỆU (PRODUCER) --- */
void push_to_buffer(SensorData data) {
    pthread_mutex_lock(&buffer_mutex);

    while (buffer_count == BUFFER_SIZE) {
        pthread_cond_wait(&cond_not_full, &buffer_mutex);
    }

    circular_buffer[buffer_head] = data;
    buffer_head = (buffer_head + 1) % BUFFER_SIZE;
    buffer_count++;

    pthread_cond_signal(&cond_not_empty);
    pthread_mutex_unlock(&buffer_mutex);
}

/* --- ĐỒNG BỘ LUỒNG: ĐỌC DỮ LIỆU (CONSUMER) --- */
SensorData pop_from_buffer(void) {
    SensorData data;

    pthread_mutex_lock(&buffer_mutex);

    while (buffer_count == 0) {
        pthread_cond_wait(&cond_not_empty, &buffer_mutex);
    }

    data = circular_buffer[buffer_tail];
    buffer_tail = (buffer_tail + 1) % BUFFER_SIZE;
    buffer_count--;

    pthread_cond_signal(&cond_not_full);
    pthread_mutex_unlock(&buffer_mutex);

    return data;
}
