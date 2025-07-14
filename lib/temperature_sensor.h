#if !defined(TEMPERATURE_SENSOR)
#define TEMPERATURE_SENSOR

#include <lvgl.h>

extern bool start_response;
extern bool sensor_response;
extern char temp_status_buffer[20];

void temp_sensor_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins);
void read_temp();
void init_temp();
#endif // TEMPERATURE_SENSOR