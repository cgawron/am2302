// Copyright 2018 Christian Gawron (christian.gawron@gmail.com)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "am2302.h"

#define ESP_INTR_FLAG_DEFAULT 0

static const char* TAG = "AM2302";

static xQueueHandle am2302_evt_queue = NULL;
static int64_t start_time;

static void IRAM_ATTR am2302_isr_handler(int64_t *pStart_time)
{
    int64_t time = esp_timer_get_time();
    time -= *pStart_time;
    xQueueSendFromISR(am2302_evt_queue, &time, NULL);
}

static void am2302_task(am2302_cb_t cb)
{
    for (;;)
    {
        int64_t time;
        int64_t last = 0;
        int64_t value = 0;
        int count = 0;
        while (count < 40)
        {
            if (xQueueReceive(am2302_evt_queue, &time, portMAX_DELAY))
            {
                // printf("AM2302 edge intr, time: %lld\n", time);

                if (time < 200 || time > 8000)
                {
                    last = time;
                    count = 0;
                    continue;
                }

                int delta = (int)(time - last);
                int bit = delta > 110 ? 1 : 0;
                value = (value << 1) | bit;
                // printf("AM2302 edge intr, i:%d, delta: %d, bit: %d value: %llx\n", count, delta, bit, value);
                last = time;
                count++;
            }
        }

        int parity = value & 0xff;
        int temperature = (value >> 8) & 0xffff;
        int humidity = (value >> 24) & 0xffff;

        int checkSum = ((temperature & 0xff) + (temperature >> 8 & 0xff) + (humidity & 0xff) + (humidity >> 8 & 0xff)) & 0xff;
        if (checkSum != parity) {
            ESP_LOGE(TAG, "parity mismatch, expected: 0x%x, actual: 0x%x", parity, checkSum);   
        }

        ESP_LOGI(TAG, "final value: %llx, parity: %x, temperature: %.1f, humidity: %.1f\n", value, parity, 0.1 * temperature, 0.1 * humidity);
        if (cb) {
            cb(0.1 * temperature, 0.1 * humidity);
        }
    }
}


static int gpio_pin;

esp_err_t read_am2302()
{
    static gpio_config_t io_conf;

    //bit mask of the pins
    io_conf.pin_bit_mask = BIT(gpio_pin);

    // == Send start signal to DHT sensor ===========

    gpio_set_direction(gpio_pin, GPIO_MODE_OUTPUT);

    // pull down for 3 ms for a smooth and nice wake up
    gpio_set_level(gpio_pin, 0);
    ets_delay_us(3000);

    // pull up for 25 us for a gentile asking for data
    gpio_set_level(gpio_pin, 1);
    ets_delay_us(25);

    start_time = esp_timer_get_time();
    
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(gpio_pin, am2302_isr_handler, &start_time);

    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_NEGEDGE;

    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;

    //enable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;

    gpio_config(&io_conf);

    return ESP_OK; 
}

esp_err_t init_am2302(int _gpio_pin, am2302_cb_t cb)
{
    gpio_pin = _gpio_pin;
    gpio_set_direction(gpio_pin, GPIO_MODE_OUTPUT);

    // pull down for 3 ms for a smooth and nice wake up
    gpio_set_level(gpio_pin, 1);

    //create a queue to handle gpio event from isr
    am2302_evt_queue = xQueueCreate(64, sizeof(int64_t));

    //start gpio task
    xTaskCreate(am2302_task, "am2302 task", 2048, cb, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    
 

    vTaskDelay(2000 / portTICK_PERIOD_MS);

    return ESP_OK;
}