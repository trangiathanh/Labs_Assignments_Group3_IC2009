#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "circular_buffer.h"
#include "sensor_thread.h"
#include "processor_thread.h"
#include "logger_thread.h"

static CircularBuffer g_cb;
static volatile int g_running = 1;

void handle_sigint(int sig) {
    (void)sig;
    g_running = 0;
    pthread_cond_broadcast(&g_cb.not_empty);
}

int main(int argc, char *argv[]) {
    int duration = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            duration = atoi(argv[i + 1]);
        }
    }

    signal(SIGINT, handle_sigint);
    cb_init(&g_cb);
    log_system_event("SMS Sensor Application Started.");

    SensorArgs sargs = { .cb = &g_cb, .running = &g_running };
    ProcessorArgs pargs = { .cb = &g_cb, .running = &g_running };

    pthread_t sensor_th, proc_th;
    pthread_create(&sensor_th, NULL, sensor_thread_func, &sargs);
    pthread_create(&proc_th, NULL, processor_thread_func, &pargs);

    if (duration > 0) {
        sleep(duration);
        g_running = 0;
        pthread_cond_broadcast(&g_cb.not_empty);
    } else {
        while (g_running) {
            sleep(1);
        }
    }

    pthread_join(sensor_th, NULL);
    pthread_join(proc_th, NULL);

    printf("[SUMMARY] produced=%zu consumed=%zu size=%zu overflow=%zu\n",
           cb_get_produced(&g_cb),
           cb_get_consumed(&g_cb),
           cb_get_size(&g_cb),
           cb_get_overflow(&g_cb));

    cb_destroy(&g_cb);
    return 0;
}
