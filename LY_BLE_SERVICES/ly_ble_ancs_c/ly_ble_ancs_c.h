#ifndef __LY_BLE_ANCS_C_H__
#define __LY_BLE_ANCS_C_H__

#define TITLE    0
#define CONTENT  1

typedef enum
{
	PUSH_NOTIFICATION_STATE_START = 0,
	PUSH_NOTIFICATION_STATE_NAME,
	PUSH_NOTIFICATION_STATE_CONTENT,
	PUSH_NOTIFICATION_STATE_STOP,
}push_notification_state_machine_t;


typedef enum
{
	PUSH_NOTIFICATION_TYPE_INCOMING_CALL = 1,
	PUSH_NOTIFICATION_TYPE_MESSAGE,
	PUSH_NOTIFICATION_TYPE_EMAIL,
	PUSH_NOTIFICATION_TYPE_WECHAT,
	PUSH_NOTIFICATION_TYPE_QQ,
	PUSH_NOTIFICATION_TYPE_FACEBOOK,
	PUSH_NOTIFICATION_TYPE_TWITTER,
	PUSH_NOTIFICATION_TYPE_WHATSAPP,
}push_notification_type_t;

typedef struct
{
	uint8_t notify_switch;
    uint8_t notify_item0;
	uint8_t notify_item1;
	uint8_t call_delay;
}push_notification_config_t;

extern void ly_ble_ancs_c_disc_handler(ble_db_discovery_evt_t * p_evt);
extern void ly_ble_ancs_c_init(void);

#endif
