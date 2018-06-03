/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "esp_types.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "esp_intr_alloc.h"
#include "driver/uart.h"

#define TIMER_DIVIDER         16  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds

void lightToggle();
void timISR(void *);
void subMain();

int status = 0;

void app_main()
{
    // xTaskCreate(subMain, "echo_test", 1024, NULL, 10, NULL);
    subMain();
}

void subMain() {
    // GPIO Configuration
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    io_conf.pin_bit_mask = 1ULL << 22;
    gpio_config(&io_conf);

    // UART Configuration
    uart_config_t ucfg;
    ucfg.baud_rate = 115200;
    ucfg.data_bits = UART_DATA_8_BITS;
    ucfg.parity = UART_PARITY_DISABLE;
    ucfg.stop_bits = UART_STOP_BITS_1;
    ucfg.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;

    uart_param_config(UART_NUM_1, &ucfg);
    uart_set_pin(UART_NUM_1, GPIO_NUM_4, GPIO_NUM_5, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_1, 1024 * 2, 0, 0, NULL, 0);

//    uint8_t * data = malloc(1024);
//    while(1) {
//        int len = uart_read_bytes(UART_NUM_1, data, 1024, 200 / portTICK_RATE_MS);
//        uart_write_bytes(UART_NUM_1, (const char *)data, len);
//    }

    // Timer Configuration
    timer_config_t tim_conf;
    tim_conf.divider = TIMER_DIVIDER;
    tim_conf.counter_dir = TIMER_COUNT_UP;
    tim_conf.counter_en = TIMER_PAUSE;
    tim_conf.alarm_en = TIMER_ALARM_EN;
    tim_conf.intr_type = TIMER_INTR_LEVEL;
    tim_conf.auto_reload = true;
    timer_init(TIMER_GROUP_0, TIMER_0, &tim_conf);

    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);

    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, 1 * TIMER_SCALE);
    timer_isr_register(TIMER_GROUP_0, TIMER_0, timISR, (void*) TIMER_0, ESP_INTR_FLAG_IRAM, NULL);
    timer_enable_intr(TIMER_GROUP_0, TIMER_0);
    timer_start(TIMER_GROUP_0, TIMER_0);



    for(;;);
}



void IRAM_ATTR timISR(void * arg) {
    int timer_idx = (int) arg;


    TIMERG0.int_clr_timers.t0 = 1;
    TIMERG0.hw_timer[timer_idx].config.alarm_en = TIMER_ALARM_EN;

    lightToggle();
}

void lightToggle() {
    char msg[80];

    if(status == 0) {
        gpio_set_level(GPIO_NUM_22, 1);
        status = 1;
        strcpy(msg, "Light on\n");
    } else {
        gpio_set_level(GPIO_NUM_22, 0);
        status = 0;
        strcpy(msg, "Light off\n");
    }
    uart_write_bytes(UART_NUM_1, (const char *)msg, strlen(msg));
}
