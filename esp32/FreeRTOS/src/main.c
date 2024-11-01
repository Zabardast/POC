
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"

#include "priority.c"
#include "sensor.c"




void app_main()
{
    // alow the serial monitor to catchup
    vTaskDelay(pdMS_TO_TICKS(500));

    // run task priority example
    // priority_threads();

    // run sensor data processing example
    sensor();

}