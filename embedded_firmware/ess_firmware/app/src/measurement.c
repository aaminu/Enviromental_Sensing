

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#include "measurement.h"

#define PRODUCER_THREAD_PRIORITY 6
#define PRODUCER_SLEEP_TIME_MS 10000
#define STACKSIZE 1024

LOG_MODULE_REGISTER(measurement, CONFIG_APP_LOG_LEVEL);

K_MSGQ_DEFINE(sensor_message_queue, sizeof(sensor_data_t), 10, 4);


static void measurement_thread_handler(void *data1, void *data2, void *data3)
{
    ARG_UNUSED(data1);
    ARG_UNUSED(data2);
    ARG_UNUSED(data3);

    int ret;
    const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(bme280));
    sensor_data_t sensor_data = {0};

    ret = device_is_ready(dev);
    if (!ret)
    {
        LOG_INF("Error: SPI device is not ready, ret: %d", ret);
        return;
    }

    while (1)
    {
        ret = sensor_sample_fetch(dev);
        if (ret < 0)
        {
            LOG_ERR("Could not fetch sample (%d)", ret);
        }

        if (sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &sensor_data.temperature))
        {
            LOG_ERR("Could not get temperature sample");
        }

        if (sensor_channel_get(dev, SENSOR_CHAN_PRESS, &sensor_data.pressure))
        {
            LOG_ERR("Could not get pressure sample");
        }

        if (sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &sensor_data.humidity))
        {
            LOG_ERR("Could not get humidity sample");
        }

        // /*push data to queue*/
        ret = k_msgq_put(&sensor_message_queue, &sensor_data, K_NO_WAIT);
        if (ret < 0)
            LOG_ERR("Error placing data in Message Queue: %d", ret);

        k_sleep(K_MSEC(PRODUCER_SLEEP_TIME_MS));
    }
}

K_THREAD_DEFINE(sensor_measure, STACKSIZE, measurement_thread_handler, NULL, NULL, NULL, PRODUCER_THREAD_PRIORITY, 0,
                0);
