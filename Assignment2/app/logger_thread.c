#include <stdio.h>
#include "logger_thread.h"

void log_system_event(const char *msg) {
    if (msg) {
        printf("[LOGGER] %s\n", msg);
    }
}
