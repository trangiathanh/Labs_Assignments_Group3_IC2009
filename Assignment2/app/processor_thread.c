#include <stdio.h>
#include <unistd.h>
#include "processor_thread.h"
#include "alert_thread.h"

void* processor_thread_func(void *arg) {
    ProcessorArgs *pargs = (ProcessorArgs*)arg;
    if (!pargs || !pargs->cb) return NULL;

    SensorData data;
    while (*(pargs->running) || cb_get_size(pargs->cb) > 0) {
        if (cb_pop(pargs->cb, &data, pargs->running) == 0) {
            check_and_trigger_alert(&data);
        }
    }
    return NULL;
}
