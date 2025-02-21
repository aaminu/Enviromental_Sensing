#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/logging/log.h>

#include "dev_bt_service.h"

LOG_MODULE_REGISTER(dev_bt_service, CONFIG_APP_LOG_LEVEL);

/********************************Static Proto***********************************/
static void ess_ccc_temp_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value);
static void ess_ccc_press_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value);
static void ess_ccc_humd_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value);
/********************Static Declaration****************************************/
static bool notify_temp_en;
static bool notify_press_en;
static bool notify_humd_en;

/* Declare Service*/
BT_GATT_SERVICE_DEFINE(
    ess_svc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_ESS),

    // Temperature
    BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_NONE,
                           NULL, NULL, NULL),

    BT_GATT_CCC(ess_ccc_temp_cfg_changed,
                BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

    // Humidity
    BT_GATT_CHARACTERISTIC(BT_UUID_HUMIDITY, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_NONE,
                           NULL, NULL, NULL),

    BT_GATT_CCC(ess_ccc_humd_cfg_changed,
                BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

    // Pressure
    BT_GATT_CHARACTERISTIC(BT_UUID_PRESSURE, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_NONE,
                           NULL, NULL, NULL),

    BT_GATT_CCC(ess_ccc_press_cfg_changed,
                BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

);

static void ess_ccc_temp_cfg_changed(const struct bt_gatt_attr *attr,
                                     uint16_t value)
{
    notify_temp_en = (value == BT_GATT_CCC_NOTIFY);
}

static void ess_ccc_press_cfg_changed(const struct bt_gatt_attr *attr,
                                      uint16_t value)
{
    notify_press_en = (value == BT_GATT_CCC_NOTIFY);
}

static void ess_ccc_humd_cfg_changed(const struct bt_gatt_attr *attr,
                                     uint16_t value)
{
    notify_humd_en = (value == BT_GATT_CCC_NOTIFY);
}


int ess_temp_notify(int16_t temp_value) 
{
    if(!notify_temp_en)
    {
        return -EACCES;
    }

    uint8_t buffer[2];
    sys_put_le16(temp_value, buffer);
    return bt_gatt_notify(NULL, &ess_svc.attrs[2], &buffer, sizeof(buffer));
}

int ess_humd_notify(uint16_t humd_value) {

    if(!notify_humd_en)
    {
        return -EACCES;
    }

    uint8_t buffer[2];
    sys_put_le16(humd_value, buffer);
    return bt_gatt_notify(NULL, &ess_svc.attrs[5], &buffer, sizeof(buffer));
}

int ess_press_notify(uint32_t press_value) {

    if(!notify_press_en)
    {
        return -EACCES;
    }
    
    uint8_t buffer[4];
    sys_put_le32(press_value, buffer);
    return bt_gatt_notify(NULL, &ess_svc.attrs[8], &buffer, sizeof(buffer));
}
