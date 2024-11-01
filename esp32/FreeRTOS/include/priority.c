// demonstrate thread priority

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void print_hello()
{
    for(;;)
    {
        printf("Hello\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void print_priority()
{
    for(;;)
    {
        printf("Hello priority\n");
        for(int i = 0; i < 5; i++)
        {
            // vTaskDelay() yields control to lower priority tasks
            for(int a = 0; a < 10000000; a++) {} // about a second sleep
            printf("priority step %d\n", i);
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void priority_threads()
{
    // demonstrate multi core specificity
    // xTaskCreate(print_hello, "hello", 2048, NULL, 1, NULL);
    // xTaskCreate(print_priority, "priority", 2048, NULL, 3, NULL);

    xTaskCreatePinnedToCore(print_hello, "hello", 2048, NULL, 1, NULL, APP_CPU_NUM);
    xTaskCreatePinnedToCore(print_priority, "priority", 2048, NULL, 3, NULL, APP_CPU_NUM);
}