#if !defined(TEMPERATURE_SENSOR)
#define TEMPERATURE_SENSOR

#include <lvgl.h>

extern bool start_response;
extern bool sensor_response;
extern char temp_status_buffer[20];
extern int32_t random_num_temp;

void read_temp();
void init_temp();
#endif // TEMPERATURE_SENSOR