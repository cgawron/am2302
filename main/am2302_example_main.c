#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../components/am2302/am2302.h"

#define AM2302_PIN 22

void callback(double temperature, double humidity) {
    printf("temperature: %.1f humidity: %.1f\n", temperature, humidity);
}

void app_main()
{
    init_am2302(AM2302_PIN, callback);

    int cnt = 0;
    while (1)
    {
        printf("cnt: %d\n", cnt++);
        read_am2302();
        vTaskDelay(30000 / portTICK_RATE_MS);
    }
}
