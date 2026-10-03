#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include "sensor_thread.h"
#include "../driver/sms_sensor.h"

typedef struct {
    const char *device_path;
    int interval_ms;
    int *running;
    CircularBuffer *cb;
} SensorArgs;

void* sensor_thread_func(void *arg) {
    SensorArgs *args = (SensorArgs *)arg;
    int fd = open(args->device_path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open sensor device");
        return NULL;
    }

    ioctl(fd, SMS_SET_INTERVAL, &args->interval_ms);

    while (*args->running) {
        char buf[128];
        ssize_t n = read(fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            SensorData data;
            int ti, td, hi, hd;
            if (sscanf(buf, "%lld,%d.%d,%d.%d", &data.timestamp_ms, &ti, &td, &hi, &hd) == 5) {
                data.temperature = (float)ti + (float)td / 100.0f;
                data.humidity = (float)hi + (float)hd / 100.0f;
                cb_push(args->cb, data);
            }
        }
    }

    close(fd);
    return NULL;
}
