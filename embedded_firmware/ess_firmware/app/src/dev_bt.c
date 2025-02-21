#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/gatt.h>

LOG_MODULE_REGISTER(dev_bt, CONFIG_APP_LOG_LEVEL);

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

/********************************Static Proto***********************************/
static void update_phy(struct bt_conn *conn);
static void update_data_length(struct bt_conn *conn);
static void exchange_func(struct bt_conn *conn, uint8_t att_err,
                          struct bt_gatt_exchange_params *params);
static void update_mtu(struct bt_conn *conn);
static void on_connected(struct bt_conn *conn, uint8_t err);
static void on_disconnected(struct bt_conn *conn, uint8_t reason);
static void on_le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout);
static void on_le_phy_updated(struct bt_conn *conn, struct bt_conn_le_phy_info *param);
static void on_le_data_len_updated(struct bt_conn *conn, struct bt_conn_le_data_len_info *info);
/*******************************************************************/

bt_addr_le_t bt_dev_addr;
static struct bt_gatt_exchange_params exchange_params;
struct bt_conn *bt_dev_conn = NULL;

// Advertisment Data
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
    BT_DATA_BYTES(BT_DATA_GAP_APPEARANCE, BT_APPEARANCE_SENSOR_TEMPERATURE & 0xff,
                  (BT_APPEARANCE_SENSOR_TEMPERATURE >> 8) & 0xff),
};

// Scan Request Response Data
static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_ESS_VAL)), // Enivorment Sensing Service
};

// Advertisment Parameters
static const struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
    BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_IDENTITY,
    BT_GAP_ADV_FAST_INT_MIN_1,
    BT_GAP_ADV_FAST_INT_MIN_1,
    NULL);

// Connection Handler
static struct bt_conn_cb dev_conn_callbacks = {
    .connected = on_connected,
    .disconnected = on_disconnected,
    .le_param_updated = on_le_param_updated,
    .le_phy_updated = on_le_phy_updated,
    .le_data_len_updated = on_le_data_len_updated,

};

int bt_space_init(void)
{
    int ret;

    ret = bt_addr_le_from_str("FF:EE:DD:CC:BB:AA", "random", &bt_dev_addr);
    if (ret)
    {
        LOG_ERR("Invalid BT address: %d", ret);
    }

    ret = bt_id_create(&bt_dev_addr, NULL);
    if (ret < 0)
    {
        LOG_ERR("Creating new ID failed: %d", ret);
    }

    ret = bt_conn_cb_register(&dev_conn_callbacks);
    if (ret)
    {
        LOG_ERR("Connection callback register failed: %d", ret);
    }

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
    LOG_INF("Advertising successfully started");

    return ret;
}

static void update_phy(struct bt_conn *conn)
{
    int ret;
    const struct bt_conn_le_phy_param pref_phy = {
        .options = BT_CONN_LE_PHY_OPT_NONE,
        .pref_rx_phy = BT_GAP_LE_PHY_2M,
        .pref_tx_phy = BT_GAP_LE_PHY_2M,
    };

    ret = bt_conn_le_phy_update(conn, &pref_phy);
    if (ret)
    {
        LOG_ERR("Failed to change PHY: %d", ret);
    }
}

static void update_data_length(struct bt_conn *conn)
{
    int ret;
    struct bt_conn_le_data_len_param dev_data_len = {
        .tx_max_len = BT_GAP_DATA_LEN_MAX,
        .tx_max_time = BT_GAP_DATA_TIME_MAX,
    };
    ret = bt_conn_le_data_len_update(bt_dev_conn, &dev_data_len);
    if (ret)
    {
        LOG_ERR("data_len_update failed: %d", ret);
    }
}

static void exchange_func(struct bt_conn *conn, uint8_t att_err,
                          struct bt_gatt_exchange_params *params)
{
    LOG_INF("MTU exchange %s", att_err == 0 ? "successful" : "failed");
    if (!att_err)
    {
        uint16_t payload_mtu = bt_gatt_get_mtu(conn) - 3; // 3 bytes used for Attribute headers.
        LOG_INF("New MTU: %d bytes", payload_mtu);
    }
}

static void update_mtu(struct bt_conn *conn)
{
    int ret;
    exchange_params.func = exchange_func;

    ret = bt_gatt_exchange_mtu(conn, &exchange_params);
    if (ret)
    {
        LOG_ERR("bt_gatt_exchange_mtu failed: %d", ret);
    }
}

static void on_le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout)
{
    double connection_interval = interval * 1.25; // in ms
    uint16_t supervision_timeout = timeout * 10;  // in ms
    LOG_INF("Connection parameters updated: connection interval %.2f ms, latency %d intervals, timeout %d ms",
            connection_interval, latency, supervision_timeout);
}

static void on_le_phy_updated(struct bt_conn *conn, struct bt_conn_le_phy_info *param)
{
    // PHY Updated
    if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_1M) {
        LOG_INF("PHY updated. New PHY: 1M");
    }
    else if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_2M) {
        LOG_INF("PHY updated. New PHY: 2M");
    }
    else if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_CODED_S8) {
        LOG_INF("PHY updated. New PHY: Long Range");
    }
}

static void on_le_data_len_updated(struct bt_conn *conn, struct bt_conn_le_data_len_info *info)
{
    uint16_t tx_len     = info->tx_max_len; 
    uint16_t tx_time    = info->tx_max_time;
    uint16_t rx_len     = info->rx_max_len;
    uint16_t rx_time    = info->rx_max_time;
    LOG_INF("Data length updated. Length %d/%d bytes, time %d/%d us", tx_len, rx_len, tx_time, rx_time);
}

static void on_connected(struct bt_conn *conn, uint8_t err)
{
    if (err)
    {
        LOG_ERR("Connection error: %d", err);
        return;
    }
    LOG_INF("Connected");
    bt_dev_conn = bt_conn_ref(conn);

    // Get connection info
    struct bt_conn_info info;
    err = bt_conn_get_info(conn, &info);
    if (err)
    {
        LOG_ERR("Failed to get connx info: %d", err);
        return;
    }

    double connection_interval = info.le.interval * 1.25; // in ms
    uint16_t supervision_timeout = info.le.timeout * 10;  // in ms
    LOG_INF("Initial Connection parameters: interval %.2f ms, latency %d intervals, timeout %d ms",
            connection_interval, info.le.latency, supervision_timeout);
            
    update_phy(bt_dev_conn);
    update_data_length(bt_dev_conn);
    update_mtu(bt_dev_conn);
}

static void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected. Reason %d", reason);
    bt_conn_unref(bt_dev_conn);

}
