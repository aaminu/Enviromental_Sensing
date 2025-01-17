#ifndef _MEASUREMENT_H_
#define _MEASUREMENT_H_

#include <zephyr/drivers/sensor.h>

typedef struct
{
    struct sensor_value temperature;
    struct sensor_value pressure;
    struct sensor_value humidity;
} sensor_data_t;

extern struct k_msgq sensor_message_queue;

#endif /*_MEASUREMENT_H_*/
