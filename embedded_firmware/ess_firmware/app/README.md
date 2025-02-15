connection intervals in 1.25ms controlled by CONFIG_BT_PERIPHERAL_PREF_MIN_INT and CONFIG_BT_PERIPHERAL_PREF_MAX_INT. Max value in zephyr is up to 3200


Peripheral preferred supervision timeout in 10ms units

It is up to user to provide valid timeout which pass required minimum value: in milliseconds it shall be larger than "(1+ Conn_Latency) * Conn_Interval_Max * 2" where Conn_Interval_Max is given in milliseconds. Range 3200 to 65534 is invalid. 65535 represents no specific value.
