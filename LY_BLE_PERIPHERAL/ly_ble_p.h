#ifndef __LY_BLE_P_H__
#define __LY_BLE_P_H__

#include "ly_ble.h"
#include "ble_db_discovery.h"

typedef struct
{
	on_ble_evt on_ble_peripheral_evt;
}ly_ble_p_t;

extern void send_data_to_remote(uint8_t *p_data, uint16_t length);
extern void print_ly_ble_p(void);
extern void adv_start(bool erase_bonds);
extern void ly_ble_p_db_disc_handler(ble_db_discovery_evt_t * p_evt);
extern void ly_ble_p_init(ly_ble_p_t *p_ly_ble_p);

#endif
