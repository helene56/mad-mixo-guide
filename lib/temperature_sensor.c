#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include "temperature_sensor.h"

#define STACKSIZE 1024
#define THREAD0_PRIORITY 15



// #define TEMP_NODE DT_ALIAS(temp)
const struct device *dht = DEVICE_DT_GET(DT_NODELABEL(dht0));
// static const struct gpio_dt_spec temp_sensor = GPIO_DT_SPEC_GET(TEMP_NODE, gpios);
// static struct gpio_callback temp_cb_data;
bool start_response = false;
bool sensor_response = false;
char temp_status_buffer[20];


uint32_t last_high_start = 0;
uint32_t high_duration = 0;



void temp_sensor_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins) 
{
    uint32_t now = k_cycle_get_32();
    high_duration = now - last_high_start;
    printk("Line was high for %u cycles\n", high_duration);
}

void read_temp()
{
    while(1)
    {
        struct sensor_value temp, humidity;
        sensor_sample_fetch(dht);
        sensor_channel_get(dht, SENSOR_CHAN_AMBIENT_TEMP, &temp);
        sensor_channel_get(dht, SENSOR_CHAN_HUMIDITY, &humidity);
        printk("Temp: %d.%06d C\n", temp.val1, temp.val2);
        snprintf(temp_status_buffer, sizeof(temp_status_buffer), "Temp: %d.%06d C\n", temp.val1, temp.val2);
       
        
        k_sleep(K_MSEC(10000));
        // printk("temp: %d\n", temp);

        // if (start_response)
        // {
        //     // int ret;
        //     // // ret = gpio_pin_configure_dt(&temp_sensor, GPIO_OUTPUT);
        //     // ret = gpio_pin_configure(temp_sensor.port, temp_sensor.pin, GPIO_OUTPUT);
        //     // if (ret < 0) 
        //     // {
        //     //     return;
        //     // }
        //     // gpio_pin_set_dt(&temp_sensor, 0);
        //     // k_busy_wait(1200);
        //     // gpio_pin_set_dt(&temp_sensor, 1);
        //     // k_busy_wait(22);
        //     // test loop
        //     gpio_pin_configure(temp_sensor.port, temp_sensor.pin, GPIO_OUTPUT);
        //     gpio_pin_set(temp_sensor.port, temp_sensor.pin, 0);   // Drive low
        //     k_busy_wait(500);  // 500us
        //     gpio_pin_configure(temp_sensor.port, temp_sensor.pin, GPIO_INPUT | GPIO_PULL_UP);  // Release high
        //     k_busy_wait(500);
        //     // sensor_response = true;
        //     // start_response = false;
        // }
        // else if (sensor_response && start_response == false)
        // {
        //     int ret;
        //     ret = gpio_pin_configure_dt(&temp_sensor, GPIO_INPUT);
        //     if (ret < 0) 
        //     {
        //         return;
        //     }
        //     ret = gpio_pin_interrupt_configure_dt(&temp_sensor, GPIO_INT_EDGE_BOTH);
        //     if (ret < 0) 
        //     {
        //         return;
        //     }

        // }
        
        
    }
    
}


void init_temp()
{
    
    // if (!device_is_ready(temp_sensor.port)) 
    // {
    //     return; 
    // }
    // gpio_init_callback(&temp_cb_data, temp_sensor_isr, BIT(temp_sensor.pin));
    // gpio_add_callback(temp_sensor.port, &temp_cb_data);
   

    if (!device_is_ready(dht)) {
        printk("DHT sensor not ready!\n");
    }	

}


K_THREAD_DEFINE(temp_read_id, STACKSIZE, read_temp, NULL, NULL, NULL,
		THREAD0_PRIORITY, 0, 0);