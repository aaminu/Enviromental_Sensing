/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <app_version.h>
#include "measurement.h"
#include "dev_bt_service.h"

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

extern int bt_space_init(void);

int main(void)
{
	printk("ESS App %s\n", APP_VERSION_STRING);

	bt_space_init(); // Initialize the Bluetooth

	sensor_data_t rcvd_sensor_data = {0};
	double pressure, temperature, humidity;

	while (1)
	{
		int ret = k_msgq_get(&sensor_message_queue, &rcvd_sensor_data, K_NO_WAIT);
		if (ret == 0)
		{

			temperature = sensor_value_to_double(&rcvd_sensor_data.temperature);
			pressure = sensor_value_to_double(&rcvd_sensor_data.pressure);
			humidity = sensor_value_to_double(&rcvd_sensor_data.humidity);

			LOG_INF("\t\t Sensor Data");
			LOG_INF("\tTemperature: \t %.2f C", temperature);
			LOG_INF("\tPressure:    \t %.2f kPa", pressure);
			LOG_INF("\tHumidity:    \t %.2f %%RH\n\n", humidity);

			// Notify Central
			ess_temp_notify((int16_t)(temperature * 100));	// 0.01C resolution
			ess_humd_notify((uint16_t)(humidity * 100));	// 0.01% RH resolution
			ess_press_notify((uint32_t)(pressure * 10000)); // 0.1Pa resolution, but already in kPa, so convert
		}

		k_sleep(K_MSEC(1000));
	}

	return 0;
}
