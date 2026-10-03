#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "logger_thread.h"

typedef struct {
    CircularBuffer *cb;
    const char *filename;
    int *running;
} LoggerArgs;

void* logger_thread_func(void *arg) {
    LoggerArgs *args = (LoggerArgs *)arg;
    FILE *f = fopen(args->filename, "w");
    if (!f) return NULL;

    SensorData data;
    while (*args->running) {
        if (cb_pop(args->cb, &data) == 0) {
            fprintf(f, "%lld,%.2f,%.2f\n", data.timestamp_ms, data.temperature, data.humidity);
            fflush(f);
        }
    }

    fclose(f);
    return NULL;
}
