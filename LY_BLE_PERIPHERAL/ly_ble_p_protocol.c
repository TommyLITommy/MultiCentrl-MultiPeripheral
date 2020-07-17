#include "sdk_common.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "ly_ble.h"
#include "ly_ble_p_protocol.h"

//Remote ble central send data to this handler
/*
	Functions:
		1.Erase (external or internal) flash by sector index(specific sector or the whole chip) through ble or uart!!!
		2.Flash (external or internal) flash by address with length
		3.Enable or disable log print
		4.Read sensor raw data(si11xx,ads129x,mems,env)
*/
void ly_ble_p_protocol_handler(uint16_t conn_handle, const uint8_t *p_data, uint16_t length)
{
	if(length >= 1)
	{
		switch(p_data[0])
		{
			case 1:
				NRF_LOG_INFO("DB Discovery");
				ly_ble_db_discovery_start(conn_handle);
				break;
			case 2:
				NRF_LOG_INFO("Long Press");
				break;
			case 3:
				NRF_LOG_INFO("Clear Screen");
				break;
			case 4:
				NRF_LOG_INFO("Reset");
				//NRF_LOG_PROCESS();
				NVIC_SystemReset();
				break;
			default:
				break;
		}
	}
}

void ly_ble_p_protocol_init(void)
{
	
}
