#ifndef ALERT_THREAD_H
#define ALERT_THREAD_H

#include "circular_buffer.h"

void check_and_trigger_alert(const SensorData *data);

#endif /* ALERT_THREAD_H */
