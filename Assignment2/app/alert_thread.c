#include <stdio.h>
#include "alert_thread.h"

void check_and_trigger_alert(const SensorData *data) {
    if (!data) return;
    if (data->temperature > 40.0f || data->humidity > 80.0f) {
        printf("  [ALERT] Warning: Temp = %.2f°C, Hum = %.2f%%\n",
               data->temperature, data->humidity);
    }
}
