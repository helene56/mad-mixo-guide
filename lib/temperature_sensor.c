#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include "temperature_sensor.h"

#define STACKSIZE 1024
#define THREAD0_PRIORITY 15

const struct device *dht = DEVICE_DT_GET(DT_NODELABEL(dht0));

char temp_status_buffer[20];
int32_t random_num_temp;

void read_temp()
{
    while (1)
    {
        struct sensor_value temp, humidity;
        sensor_sample_fetch(dht);
        sensor_channel_get(dht, SENSOR_CHAN_AMBIENT_TEMP, &temp);
        sensor_channel_get(dht, SENSOR_CHAN_HUMIDITY, &humidity);
        printk("Temp: %d.%06d C\n", temp.val1, temp.val2);
        random_num_temp = (temp.val1 * 10) + (temp.val2 / 1000000);
        snprintf(temp_status_buffer, sizeof(temp_status_buffer), "Temp: %d.%06d C\n", temp.val1, temp.val2);

        k_sleep(K_MSEC(10000));
    }
}

void init_temp()
{
    if (!device_is_ready(dht))
    {
        printk("DHT sensor not ready!\n");
    }
}

K_THREAD_DEFINE(temp_read_id, STACKSIZE, read_temp, NULL, NULL, NULL,
                THREAD0_PRIORITY, 0, 0);