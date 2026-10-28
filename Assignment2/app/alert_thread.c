#include <stdio.h>
#include <unistd.h>
#include "alert_thread.h"

void* alert_thread_func(void *arg) {
    AlertArgs *aargs = (AlertArgs*)arg;
    if (!aargs || !aargs->cb) return NULL;

    while (*(aargs->running)) {
        usleep(200000);
    }
    return NULL;
}
