/*
 * sensorctl.h
 *
 *  Created on: Jun 11, 2018
 *      Author: chili
 */

#ifndef MAIN_SENSORCTL_H_
#define MAIN_SENSORCTL_H_

#include "driver/adc.h"
#include "esp_adc_cal.h"

typedef struct sensor_dat_t {
    uint8_t len; // actual length of currently written data
    uint8_t dat[64]; // data array of the MAXIMUM allowable length
} sensor_dat_t;

// Transmission
void bt_setup();

// Sensor readings
esp_adc_cal_characteristics_t * adc_setup();
void adc_read_update(esp_adc_cal_characteristics_t * config);
void bluetoothify(const uint32_t data[], int data_len);
void temp_humidity();

#endif /* MAIN_SENSORCTL_H_ */
