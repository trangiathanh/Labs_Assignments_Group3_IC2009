#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "sensor_thread.h"

void* sensor_thread_func(void *arg) {
    SensorArgs *sargs = (SensorArgs*)arg;
    if (!sargs || !sargs->cb) return NULL;

    int fd = open("/dev/sms_sensor", O_RDONLY);
    char buf[64];

    while (*(sargs->running)) {
        if (fd >= 0) {
            ssize_t n = read(fd, buf, sizeof(buf) - 1);
            if (n > 0) {
                buf[n] = '\0';
                SensorData data;
                unsigned long ms;
                if (sscanf(buf, "%lu,%f,%f", &ms, &data.temperature, &data.humidity) >= 3) {
                    data.timestamp = ms;
                    cb_push(sargs->cb, data);
                }
            }
        }
        usleep(500000);
    }
    if (fd >= 0) close(fd);
    return NULL;
}
