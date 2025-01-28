#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/gatt.h>


LOG_MODULE_REGISTER(bt_space, CONFIG_APP_LOG_LEVEL);

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)


// Advertisment Data
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
    BT_DATA_BYTES(BT_DATA_GAP_APPEARANCE, BT_APPEARANCE_SENSOR_TEMPERATURE & 0xff,
                  (BT_APPEARANCE_SENSOR_TEMPERATURE >> 8) & 0xff),
};

// Scan Request Response + Data
static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_ESS_VAL)), // Enivorment Sensing Service
};

static const struct bt_le_adv_param *adv_param =
    BT_LE_ADV_PARAM(BT_LE_ADV_OPT_NONE, 800, 802, NULL);



int bt_space_init(void)
{
    int ret;

    // Enable BT Synchronously
    ret = bt_enable(NULL);
    if (ret)
    {
        LOG_ERR("Bluetooth Init Failed: %d", ret);
        return -1;
    }
    LOG_INF("Bluetooth Initialized\n");

    ret = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));

    if (ret)
    {
        LOG_ERR("Advertising failed to start: %d\n", ret);
        return -1;
    }

    return ret;
}
