// demonstrate sensor data usage from multiple tasks

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

int sensor_a;
int sensor_b;
SemaphoreHandle_t data_sem_a;
SemaphoreHandle_t data_sem_b;


void task_sensor_a_input()
{
    for(;;)
    {
        printf("sensor a\n");
        for(int i = 0; i < 5; i++)
        {
            // vTaskDelay() yields control to lower priority tasks
            // for(int a = 0; a < 10000000; a++){} // about a second sleep
            for(int a = 0; a < 100000; a++){} // < second sleep

            // use i as sensor input
            sensor_a = i;
            if(i == 4)
            {
                // sensor_a = i;
                xSemaphoreGive(data_sem_a);
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void task_sensor_b_input()
{
    for(;;)
    {
        printf("sensor b\n");
        for(int i = 0; i < 5; i++)
        {
            // vTaskDelay() yields control to lower priority tasks
            // for(int b = 0; b < 10000000; b++){} // about a second sleep
            for(int b = 0; b < 100000; b++){} // < second sleep

            // use i as sensor input
            sensor_b = i;
            if(i == 3)
            {
                // sensor_b = i;
                xSemaphoreGive(data_sem_b);
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void task_compute()
{
    for(;;)
    {
        // print sensor a
        if( xSemaphoreTake(data_sem_a, 0) == pdTRUE )
        {
            printf("sensor_a : %i\n", sensor_a);
        }

        // print sensor b
        if( xSemaphoreTake(data_sem_b, 0) == pdTRUE )
        {
            printf("sensor_b : %i\n", sensor_b);
        }
    }
}

void sensor()
{
    data_sem_a = xSemaphoreCreateBinary();
    data_sem_b = xSemaphoreCreateBinary();

    // xTaskCreate(task_sensor_a_input, "sensor_a", 1024, NULL, 3, NULL);
    // xTaskCreate(task_sensor_b_input, "sensor_b", 1024, NULL, 3, NULL);

    xTaskCreatePinnedToCore(task_sensor_a_input, "sensor_a", 1024, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(task_sensor_b_input, "sensor_b", 1024, NULL, 3, NULL, 1);

    xTaskCreatePinnedToCore(task_compute, "compute", 2048, NULL, 5, NULL, 0);
}