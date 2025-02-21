#ifndef _DEV_BT_SERVICE_H_
#define _DEV_BT_SERVICE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <zephyr/types.h>

/**
 * @brief Notify connected Central/Remote Device with Temperature
 *
 * @param temp_value temperature value of the sensor
 *
 * @retval 0 if successful otherwise < 0
 */
int ess_temp_notify(int16_t temp_value);

/**
 * @brief Notify connected Central/Remote Device with Humidity
 *
 * @param humd_value humidity value of the sensor
 *
 * @retval 0 if successful otherwise < 0
 */
int ess_humd_notify(uint16_t humd_value);

/**
 * @brief Notify connected Central/Remote Device with Pressure
 *
 * @param press_value pressure value of the sensor
 *
 * @retval 0 if successful otherwise < 0
 */
int ess_press_notify(uint32_t press_value);

#ifdef __cplusplus
}
#endif

#endif
