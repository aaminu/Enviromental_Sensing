/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <app_version.h>
#include "measurement.h"

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

int main(void)
{
	printk("ESS App %s\n", APP_VERSION_STRING);

	sensor_data_t rcvd_sensor_data = {0};

	while (1)
	{
		int ret = k_msgq_get(&sensor_message_queue, &rcvd_sensor_data, K_NO_WAIT);
		if (ret == 0)
		{
			LOG_INF("\t\t Sensor Data");
			LOG_INF("\tTemperature: \t %8d.%3d C",
					rcvd_sensor_data.temperature.val1, rcvd_sensor_data.temperature.val2 / 1000);
			LOG_INF("\tPressure:    \t %8d.%3d Pa",
					rcvd_sensor_data.pressure.val1, rcvd_sensor_data.pressure.val2 / 1000);
			LOG_INF("\tHumidity:    \t %8d.%3d %%RH\n\n",
					rcvd_sensor_data.humidity.val1, rcvd_sensor_data.humidity.val2 / 1000);
		}

		k_sleep(K_MSEC(1000));
	}

	return 0;
}
