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

#ifndef _AM2302_H_
#define _AM2302_H_

#include <esp_err.h>

typedef void (*am2302_cb_t)(double temperature, double humidity);

/**
 * @brief Initialize the AM2302 driver 
 *
 * @param  gpio_pin GPIO number of the AM2302 SDA pin
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *
 */
esp_err_t init_am2302(int gpio_pin);

/**
 * @brief Start measurement of humidity and temperature on the AM2302 
 *
 * @param callback Pointer to a callback function receiving the results of subsequent measurements
 * 
 * @return
 *     - ESP_OK success
 *     - ESP_FAIL Operation fail
 *
 */
esp_err_t read_am2302(am2302_cb_t cb);

#endif