#include "sdk_common.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "drv_flash.h"
#include "crc16.h"
#include "ly_ble.h"
#include "ly_ble_p_protocol.h"

static void ly_ble_p_protocol_gid_debug_handler(uint16_t conn_handle, const uint8_t *p_data, uint16_t length)
{
	switch(p_data[BLE_PROTOCOL_COMMAND_ID_FIELD_OFFSET])
	{
		case GID_DEBUG_CID_SHORT_PRESS:
			NRF_LOG_INFO("short press");
			break;
		case GID_DEBUG_CID_LONG_PRESS:
			NRF_LOG_INFO("long press");
			break;
		case GID_DEBUG_CID_SYSTEM_RESET:
			NVIC_SystemReset();
			break;
		case GID_DEBUG_CID_DATABASE_DISCOVERY:
			NRF_LOG_INFO("db discovery");
			ly_ble_db_discovery_start(conn_handle);
			break;
		default:
			break;
	}
}

static void ly_ble_p_protocol_gid_flash_handler(uint16_t conn_handle, const uint8_t *p_data, uint16_t length)
{
	uint16_t payload_length = 0;
	uint8_t flash_id;
	uint32_t flash_start_address;
	uint32_t flash_operation_len;

	payload_length = uint16_decode(&p_data[BLE_PROTOCOL_PAYLOAD_LENGTH_FIELD_OFFSET]);

	if(payload_length < 9)
	{
		return;
	}

	flash_id = p_data[BLE_PROTOCOL_PAYLOAD_OFFSET];
	flash_start_address = uint32_decode(&p_data[BLE_PROTOCOL_PAYLOAD_OFFSET + 1]);

	if(flash_start_address < 0x200000 || flash_start_address > 0x400000)
	{
		NRF_LOG_INFO("start address error");
		return;
	}
	
	switch(p_data[BLE_PROTOCOL_COMMAND_ID_FIELD_OFFSET])
	{
		case GID_FLASH_CID_ERASE:
			{
				uint8_t sector_count;
				
				NRF_LOG_INFO("GID_FLASH_CID_READ");

				//if(flash_start_address < 0x200000 || flash_start_address > 0x400000 || ((flash_start_address % FLASH_SECTOR_SIZE) != 0))
				if((flash_start_address % FLASH_SECTOR_SIZE) != 0)
				{
					NRF_LOG_INFO("start address error");
					return;
				}

				flash_operation_len = uint32_decode(&p_data[BLE_PROTOCOL_PAYLOAD_OFFSET + 5]);

				if((flash_operation_len % FLASH_SECTOR_SIZE) != 0)
				{
					NRF_LOG_INFO("erase length error");
					return;
				}
				
				sector_count = flash_operation_len / FLASH_SECTOR_SIZE;

				if(sector_count > FLASH_TOTAL_SECOTR_COUNT)
				{
					NRF_LOG_INFO("sector_count > %d", FLASH_TOTAL_SECOTR_COUNT);
					return;
				}

				for(uint16_t i = 0; i < sector_count; i++)
				{
					drv_flash_erase_sector(flash_start_address +i * FLASH_SECTOR_SIZE);
				}
			}
			break;
		case GID_FLASH_CID_WRITE:
			{
				flash_operation_len = uint32_decode(&p_data[BLE_PROTOCOL_PAYLOAD_OFFSET + 1 + 4]);
				NRF_LOG_INFO("GID_FLASH_CID_WRITE, address:%x,length:%x", flash_start_address, flash_operation_len);
				//NRF_LOG_HEXDUMP_INFO(&p_data[BLE_PROTOCOL_PAYLOAD_OFFSET + 5], flash_operation_len);
				//drv_flash_write(flash_start_address, (uint8_t*)(&p_data[BLE_PROTOCOL_PAYLOAD_OFFSET + 9]), flash_operation_len);
			}
			break;
		case GID_FLASH_CID_READ:
			{
				flash_operation_len = uint32_decode(&p_data[BLE_PROTOCOL_PAYLOAD_OFFSET + 5]);
				//There should pay more attention, If the length to be read is too large, it has to split the data into small pieces!
				NRF_LOG_INFO("GID_FLASH_CID_READ");
			}
			break;
		default:
			break;
	}
}

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
	uint16_t crc16_01 = 0, crc16_02 = 0;

	if(length < BLE_PROTOCOL_MIN_SIZE)
	{
		NRF_LOG_INFO("length < %d", BLE_PROTOCOL_MIN_SIZE);
		return;
	}
	
	crc16_01 += p_data[length - 2] << 0;
	crc16_01 += p_data[length - 1] << 8;

	crc16_02 = crc16_compute(p_data, length - 2, NULL);

	NRF_LOG_INFO("crc16_01:%d, crc16_02:%d", crc16_01, crc16_02);
	//if(crc16_01 != crc16_02)
	if(0)
	{	
		NRF_LOG_INFO("crc16 check error");
		return;
	}

	switch(p_data[BLE_PROTOCOL_GROUP_ID_FIELD_OFFSET])
	{
		case GROUP_ID_DEBUG:
			ly_ble_p_protocol_gid_debug_handler(conn_handle, p_data, length);
			break;
		case GROUP_ID_FLASH:
			ly_ble_p_protocol_gid_flash_handler(conn_handle, p_data, length);
			break;
		default:
			break;
	}


	NRF_LOG_INFO("\r\n");
}

void ly_ble_p_protocol_init(void)
{
	
}
