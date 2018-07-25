/*
 * Test sensor, read data from ADC based on potentiometer.
 * Using ADC1 Channel 6, 11db Attenuation to get a FS of 3.9V.
 * Use multisampling for greater accuracy
 */

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_spi_flash.h"

#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "sensorctl.h"
#include "DHT11.h"

#define SAMPLES 16 //16x multisampling/averaging


extern sensor_dat_t rsp_dat;
extern int DHT_DATA[];

void temp_humidity() {
    uint32_t dat[2] = {0}; // Give temp in C, humidity
    //printf("Temp %d, Humid %d\n", DHT_DATA[1], DHT_DATA[2]);

    getData();
    vTaskDelay(10/portTICK_PERIOD_MS);
    dat[0] = (uint32_t) DHT_DATA[1]; // temp in F
    dat[1] = (uint32_t) DHT_DATA[2]; // humidity (%)

    bluetoothify(dat, 2);
}

esp_adc_cal_characteristics_t * adc_setup() {
    // Configuration variables
    static esp_adc_cal_characteristics_t *adc_config;
    static const adc_atten_t atten = ADC_ATTEN_DB_11; // set input attenuation to 11dB
    static const adc_unit_t unit = ADC_UNIT_1;

    adc_config = calloc(1, sizeof(esp_adc_cal_characteristics_t));

    // Setup adc with desired traits
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten,
            ADC_WIDTH_BIT_12, 1100, adc_config);

    return adc_config;
}

void adc_read_update(esp_adc_cal_characteristics_t * config) {
    uint32_t reading1 = 0, reading2 = 0;
    // Sample & average
    for(int i = 0; i < SAMPLES; i++) {
        reading1 += adc1_get_raw((adc_channel_t)ADC_CHANNEL_6);
        reading2 += adc1_get_raw((adc_channel_t)ADC_CHANNEL_7);
    }
    reading1 /= SAMPLES; // Avg data
    reading2 /= SAMPLES;

    uint32_t milis1 = esp_adc_cal_raw_to_voltage(reading1, config);
    uint32_t milis2 = esp_adc_cal_raw_to_voltage(reading2, config);

    uint32_t dat[2] = {0};
    dat[0] = milis1;
    dat[1] = milis2;


    bluetoothify(dat, 2);
}

void bluetoothify(const uint32_t data[], int data_len) { // TODO: Add name, type parameters, better protocol
    // Allocate space
    uint8_t serialized[64] = {0};
    int j = 0; // Need external counter
    /*uint32_t curr;
    uint32_t mask = 0xFF;*/ // Bottom 8 bits


    /*do { // Go through all data
        curr = data[j];
        //printf("curr = %u\n", curr);
        for(int i = 0; i < 4; i++) { // Shift and mask
            serialized[i] = curr & mask;
            curr >>= 8;
        }
        j++;
    } while(j < data_len);*/

    uint8_t * p = (uint8_t *) data;
    uint8_t * last = p + (32 * data_len);
    while(p < last) {
        serialized[j++] = *p;
        p++;
    }

    //printf("%u %u\n", serialized[0], serialized[4]);

    rsp_dat.len = 4 * data_len;
    memcpy(rsp_dat.dat, serialized, rsp_dat.len);
}
