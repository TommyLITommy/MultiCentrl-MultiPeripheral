#ifndef __LY_BLE_C_H__
#define __LY_BLE_C_H__

#include "ly_ble.h"
#include "ble_db_discovery.h"

typedef struct
{
	on_ble_evt on_ble_central_evt;
}ly_ble_c_t;

extern void print_ly_ble_c(void);
extern void ly_ble_c_db_disc_handler(ble_db_discovery_evt_t * p_evt);
extern void scan_start(void);
extern void scan_stop(void);
extern void ly_ble_c_init(ly_ble_c_t *p_ly_ble_c);

#endif
