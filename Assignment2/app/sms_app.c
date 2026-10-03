#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "circular_buffer.h"
#include "sensor_thread.h"
#include "logger_thread.h"

static int running = 1;

void handle_sigint(int sig) {
    (void)sig;
    running = 0;
}

int main(void) {
    signal(SIGINT, handle_sigint);

    CircularBuffer cb;
    cb_init(&cb);

    typedef struct { const char *path; int interval; int *run; CircularBuffer *buf; } SArgs;
    SArgs s_args = { "/dev/sms_sensor", 100, &running, &cb };

    typedef struct { CircularBuffer *buf; const char *file; int *run; } LArgs;
    LArgs l_args = { &cb, "sensor_output.log", &running };

    pthread_t t_sensor, t_logger;
    pthread_create(&t_sensor, NULL, sensor_thread_func, &s_args);
    pthread_create(&t_logger, NULL, logger_thread_func, &l_args);

    while (running) {
        usleep(100000);
    }

    pthread_join(t_sensor, NULL);
    pthread_join(t_logger, NULL);

    cb_destroy(&cb);
    printf("App exited cleanly.\n");
    return 0;
}
