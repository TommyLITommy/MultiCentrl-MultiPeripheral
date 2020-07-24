#ifndef __LY_BLE_P_PROTOCOL_H__
#define __LY_BLE_P_PROTOCOL_H__

#define BLE_PROTOCOL_MIN_SIZE						7


#define BLE_PROTOCOL_SEQUENCE_FIELD_OFFSET 			0
#define BLE_PROTOCOL_GROUP_ID_FIELD_OFFSET			1
#define BLE_PROTOCOL_COMMAND_ID_FIELD_OFFSET		2
#define BLE_PROTOCOL_PAYLOAD_LENGTH_FIELD_OFFSET	3
#define BLE_PROTOCOL_PAYLOAD_OFFSET					5

typedef enum
{
	GROUP_ID_DEBUG = 0x06,
	GROUP_ID_FLASH = 0x07,
}gid_id_t;

typedef enum
{
	GID_DEBUG_CID_SHORT_PRESS,
	GID_DEBUG_CID_LONG_PRESS,
	GID_DEBUG_CID_SYSTEM_RESET,
	GID_DEBUG_CID_DATABASE_DISCOVERY,
}gid_debug_cid_t;

typedef enum
{
	GID_FLASH_CID_ERASE = 0,
	GID_FLASH_CID_WRITE,
	GID_FLASH_CID_READ,
}gid_flash_cid_t;


enum
{	
	FLASH_ID_INTERNAL = 0,
	FLASH_ID_EXTERNAL,
};
	
extern void ly_ble_p_protocol_handler(uint16_t conn_handle, const uint8_t *p_data, uint16_t length);

#endif
