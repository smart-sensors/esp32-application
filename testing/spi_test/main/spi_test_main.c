
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_spi_flash.h"

#include "esp_types.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"

#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"

#define MOSI_PIN 13
#define SCLK_PIN 14
#define LCD_SPEED_HZ 1500

const char CLEAR[2] = {0xFE, 0x51};



spi_device_handle_t hspi_enable() {

    esp_err_t ret;
    spi_device_handle_t spi;
    // Make bus
    spi_bus_config_t buscfg = {
            .miso_io_num = -1,
            .mosi_io_num = MOSI_PIN,
            .sclk_io_num = SCLK_PIN,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = 0
    };

    // Add device to bus (NO Slave Select line, only 1 device)
    spi_device_interface_config_t lcdcfg = {
            .clock_speed_hz = LCD_SPEED_HZ,
            .mode = 2,
            .queue_size = 7,
            .spics_io_num = -1,
            .command_bits = 0
    };

    ret = spi_bus_initialize(HSPI_HOST, &buscfg, 0);
    ESP_ERROR_CHECK(ret);

    ret = spi_bus_add_device(HSPI_HOST, &lcdcfg, &spi);
    ESP_ERROR_CHECK(ret);


    return spi;

}

void lcd_write(spi_device_handle_t lcd, char * msg) {
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = strlen(msg) * 8;
    t.tx_buffer = (uint8_t *) msg;
    t.user = (void *)0;

    esp_err_t ret = spi_device_transmit(lcd, &t);
    assert(ret == ESP_OK);
}

void clear_lcd(spi_device_handle_t lcd) {
    char msg[2];
    msg[0] = 0xFE;
    msg[1] = 0x51;

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 16;
    t.tx_buffer = msg;
    t.user = (void *)0;

    esp_err_t ret = spi_device_transmit(lcd, &t);
    assert(ret == ESP_OK);

}

esp_err_t eventHandler(void * ctx, system_event_t * event) {
    return ESP_OK;
}

void wifiSetup() {
    nvs_flash_init();
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(eventHandler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

//    wifi_config_t sta_config;
//    sta_config.sta.ssid = "NEKOGEAR";
//    sta_config.sta.password = "wakaranai";
//    sta_config.sta.bssid_set = false;

    wifi_config_t sta_config = {
            .sta = {
                    .ssid = "NEKOGEAR",
                    .password = "wakaranai",
                    .bssid_set = false
            }
    };

    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());


}

void on_indication() {
    gpio_config_t io_conf = {
            .intr_type = GPIO_PIN_INTR_DISABLE,
            .mode = GPIO_MODE_OUTPUT,
            .pull_down_en = 0,
            .pull_up_en = 0,
            .pin_bit_mask = 1ULL << 32
    };
    gpio_config(&io_conf);
    gpio_set_level(GPIO_NUM_32, 1);

}

void app_main() {
    on_indication();
	spi_device_handle_t display = hspi_enable();
	wifiSetup();
	lcd_write(display, "Connected.");

	gpio_set_direction(GPIO_NUM_15, GPIO_MODE_OUTPUT);
	int level = 0;
	for(;;) {
	    gpio_set_level(GPIO_NUM_15, level);
	    level = !level;
	    vTaskDelay(300 / portTICK_PERIOD_MS);
	}

}


