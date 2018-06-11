/*
 * Test sensor, read data from ADC based on potentiometer.
 * Using ADC1 Channel 6, 11db Attenuation to get a FS of 3.9V.
 * Use multisampling for greater accuracy
 */

#include <string.h>

#include "esp_system.h"
#include "esp_spi_flash.h"

#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "sensorctl.h"

#define SAMPLES 16 //16x multisampling/averaging

const char NAME[3] = "TST";
const char TYPE[3] = "VMM";

extern sensor_dat_t rsp_dat;

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
    uint32_t reading = 0;
    // Sample & average
    for(int i = 0; i < SAMPLES; i++) {
        reading += adc1_get_raw((adc_channel_t)ADC_CHANNEL_6);
    }
    reading /= SAMPLES; // Avg data
    uint32_t milis = esp_adc_cal_raw_to_voltage(reading, config);

    bluetoothify(milis);
}

void bluetoothify(uint32_t data) { // TODO: Add name, type parameters, better protocol
    // Allocate space
    uint8_t serialized[64];
    int i = 0; // Need external counter
    uint32_t mask = 0xFF;

    for( ; i < 3; i++) {
        serialized[i] = NAME[i];
    }

    serialized[i++] = '_';

    do { // Shift and mask
        serialized[i++] = data & mask;
        data >>= 8;
    } while(data != 0);

    serialized[i++] = '_';

    for(int j = 0; j < 3; j++, i++) {
        serialized[i] = NAME[j];
    }

    rsp_dat.len = i;
    memcpy(rsp_dat.dat, serialized, rsp_dat.len);
}
