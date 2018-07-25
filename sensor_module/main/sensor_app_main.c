/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

// C Std Lib includes
#include <stdio.h>

// ESP32 System includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_types.h"
//#include "driver/adc.h"
//#include "esp_adc_cal.h"

// Other includes
#include "sensorctl.h"
#include "DHT11.h"


sensor_dat_t rsp_dat = {
        .len = 64,
        .dat = {0}
};

void sense(void * empty_arg) {
    esp_adc_cal_characteristics_t * config = adc_setup(); // Our dummy "sensor"

    for(;;) {
        adc_read_update(config);
        //temp_humidity();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    bt_setup();
    //esp_adc_cal_characteristics_t * config = adc_setup(); // Our dummy "sensor"
                 // Using ADC for testing, reads voltage off of potentiometer
    setDHTPin(18);
    xTaskCreate(
            sense,
            "MAIN_TASK",
            4000, // stack size
            NULL,
            1, //Priority, slightly higher than idle
            NULL);
}
