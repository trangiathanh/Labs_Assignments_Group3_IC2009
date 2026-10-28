#include <stdio.h>
#include <unistd.h>
#include "processor_thread.h"

void* processor_thread_func(void *arg) {
    ProcessorArgs *pargs = (ProcessorArgs*)arg;
    if (!pargs || !pargs->cb) return NULL;

    SensorData data;
    while (*(pargs->running) || cb_get_size(pargs->cb) > 0) {
        if (cb_pop(pargs->cb, &data, pargs->running) == 0) {
            if (data.temperature > 40.0f || data.humidity > 80.0f) {
                printf("  [ALERT] Warning: Temp = %.2f°C, Hum = %.2f%%\n",
                       data.temperature, data.humidity);
            }
        }
    }
    return NULL;
}
