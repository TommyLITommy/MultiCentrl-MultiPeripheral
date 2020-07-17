#ifndef __LY_BLE_H__
#define __LY_BLE_H__

#include "ble.h"

/**@brief   Priority of the application BLE event handler.
 * @note    You shouldn't need to modify this value.
 */
#define APP_BLE_OBSERVER_PRIO           	3
#define APP_BLE_CONN_CFG_TAG                1

typedef void(*on_ble_evt)(ble_evt_t const * p_ble_evt);
extern uint16_t lastest_conn_handle;
extern void ly_ble_db_discovery_start(uint16_t conn_handle);
extern void ly_ble_init(void);

#endif
