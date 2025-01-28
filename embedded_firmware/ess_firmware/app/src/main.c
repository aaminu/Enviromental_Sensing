/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <app_version.h>
#include "measurement.h"

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

extern int bt_space_init(void);

int main(void)
{
	printk("ESS App %s\n", APP_VERSION_STRING);

	bt_space_init(); // Initialize the Bluetooth

	sensor_data_t rcvd_sensor_data = {0};

	while (1)
	{
		int ret = k_msgq_get(&sensor_message_queue, &rcvd_sensor_data, K_NO_WAIT);
		if (ret == 0)
		{
			LOG_INF("\t\t Sensor Data");
			LOG_INF("\tTemperature: \t %.2f C", sensor_value_to_double(&rcvd_sensor_data.temperature));
			LOG_INF("\tPressure:    \t %.2f Pa", sensor_value_to_double(&rcvd_sensor_data.pressure));
			LOG_INF("\tHumidity:    \t %.2f %%RH\n\n", sensor_value_to_double(&rcvd_sensor_data.humidity));
		}

		k_sleep(K_MSEC(1000));
	}

	return 0;
}
