
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"

#include "priority.c"




void app_main()
{
    // alow the serial monitor to catchup
    vTaskDelay(pdMS_TO_TICKS(500));

    // run example
    priority_threads();

}