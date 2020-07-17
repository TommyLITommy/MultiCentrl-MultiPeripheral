#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble_hci.h"
#include "ble_gap.h"
#include "ble_err.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "app_timer.h"
#include "bsp_btn_ble.h"
#include "fds.h"
#include "nrf_delay.h"
#include "app_scheduler.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "ble_db_discovery.h"
//#include "nrf_ble_gatts_c.h"
#include "nrf_ble_ancs_c.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "ly_ble_ancs_c.h"

/*********************************************************

Notification
ANCS_C:INFO:Event:       Added
ANCS_C:INFO:Category ID: Social
ANCS_C:INFO:FILE = ..\..\..\ancs_c\ancs_c.c, FUNC = ancs_c_request_attribute
ANCS_C:INFO:App Identifier: com.tencent.mqq
ANCS_C:INFO:Title: QQ
ANCS_C:INFO:Subtitle: (N/A)
ANCS_C:INFO:Message: 涓存椂鍙风爜:hello
ANCS_C:INFO:Message Size: 18


Notification
ANCS_C:INFO:Event:       Added
ANCS_C:INFO:Category ID: Social
ANCS_C:INFO:FILE = ..\..\..\ancs_c\ancs_c.c, FUNC = ancs_c_request_attribute
ANCS_C:INFO:App Identifier: com.tencent.xin
ANCS_C:INFO:Title: 璧宠党楦ｄ汉锛屾湵涔夎緣
ANCS_C:INFO:Subtitle: (N/A)
ANCS_C:INFO:Message: helloworld
ANCS_C:INFO:Message Size: 10

Notification
ANCS_C:INFO:Event:       Added
ANCS_C:INFO:Category ID: Incoming Call
ANCS_C:INFO:FILE = ..\..\..\ancs_c\ancs_c.c, FUNC = ancs_c_request_attribute
BLE_ANCS_C:INFO:BLE_GATTC_EVT_HVX
ANCS_C:INFO:App Identifier: com.apple.mobilephone
ANCS_C:INFO:___________app_identifer = com.apple.mobilephone
ANCS_C:INFO:Title: 鈥?52 1773 0021鈥
ANCS_C:INFO:Subtitle: 骞夸笢 娣卞湷
ANCS_C:INFO:Message: 鍛煎叆鐢佃瘽
ANCS_C:INFO:Message Size: 12
BLE_ANCS_C:INFO:BLE_GATTC_EVT_HVX
BLE_ANCS_C:INFO:BLE_GATTC_EVT_HVX
ANCS_C:INFO:
Notification
ANCS_C:INFO:Event:       Removed
ANCS_C:INFO:Category ID: Incoming Call
ANCS_C:INFO:FILE = ..\..\..\ancs_c\ancs_c.c, FUNC = ancs_c_request_attribute
BLE_ANCS_C:INFO:BLE_GATTC_EVT_HVX
ANCS_C:INFO:
Notification
ANCS_C:INFO:Event:       Added
ANCS_C:INFO:Category ID: Missed Call
ANCS_C:INFO:FILE = ..\..\..\ancs_c\ancs_c.c, FUNC = ancs_c_request_attribute
ANCS_C:INFO:Error: Parameter does not refer to an existing object on the Notification Provider.
BLE_ANCS_C:INFO:BLE_GATTC_EVT_HVX
ANCS_C:INFO:App Identifier: com.apple.mobilephone
ANCS_C:INFO:___________app_identifer = com.apple.mobilephone
ANCS_C:INFO:Title: 鈥?52 1773 0021鈥
ANCS_C:INFO:Subtitle: (N/A)
ANCS_C:INFO:Message: 鏈帴鏉ョ數
ANCS_C:INFO:Message Size: 12

Notification
ANCS_C:INFO:Event:       Added
ANCS_C:INFO:Category ID: Social
ANCS_C:INFO:FILE = ..\..\..\ancs_c\ancs_c.c, FUNC = ancs_c_request_attribute
BLE_ANCS_C:INFO:BLE_GATTC_EVT_HVX
ANCS_C:INFO:App Identifier: com.apple.MobileSMS
ANCS_C:INFO:___________app_identifer = com.apple.MobileSMS
ANCS_C:INFO:Title: 152 1773 0021
ANCS_C:INFO:Subtitle: (N/A)
ANCS_C:INFO:Message: henne
ANCS_C:INFO:Message Size: 5

//QQ Email
Notification
ANCS_C:INFO:Event:       Added
ANCS_C:INFO:Category ID: Other
ANCS_C:INFO:FILE = ..\..\..\ancs_c\ancs_c.c, FUNC = ancs_c_request_attribute
BLE_ANCS_C:INFO:BLE_GATTC_EVT_HVX
ANCS_C:INFO:App Identifier: com.tencent.xin
ANCS_C:INFO:___________app_identifer = com.tencent.xin
ANCS_C:INFO:Title: 寰俊
ANCS_C:INFO:Subtitle: (N/A)
ANCS_C:INFO:Message: "涓存椂鍙风爜" <3528667896@qq.com>:how are you
ANCS_C:INFO:Message Size: 46


//Facebook
Notification
ANCS_C:INFO:Event:       Added
ANCS_C:INFO:Category ID: News
ANCS_C:INFO:FILE = ..\..\..\ancs_c\ancs_c.c, FUNC = ancs_c_request_attribute
ANCS_C:INFO:App Identifier: com.facebook.Facebook
ANCS_C:INFO:___________app_identifer = com.facebook.Facebook
ANCS_C:INFO:Title: Facebook
ANCS_C:INFO:___________title = Facebook
ANCS_C:INFO:Subtitle: (N/A)
ANCS_C:INFO:Message: 鏉庢磱鍦ㄤ綘鐨勬椂闂寸嚎鍙戝笘
ANCS_C:INFO:Message Size: 30



没有存号码的incoming call
:DEBUG:Attribute finished!
:INFO:+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
:INFO:attr_id = 1
:INFO:Title: 鈥?51 7924 0648鈥
:INFO:title length = 19
:INFO:
E2 80 AD 31 35 31 20 37 39 32 34 20 30 36 34 38  ??51 7924 0648
E2 80 AC                                         ??
:INFO:___________title = 鈥?51 7924 0648鈥
:DEBUG:Attribute ID 2
:DEBUG:Attribute LEN 0
:INFO:+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
:INFO:attr_id = 2
:INFO:Subtitle: (N/A)
:DEBUG:Attribute ID 3
:DEBUG:Attribute finished!
:INFO:+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
:INFO:attr_id = 3
:INFO:Message: Incoming Call
:INFO:message length = 13
:INFO:
49 6E 63 6F 6D 69 6E 67 20 43 61 6C 6C           Incoming Call

有存号码的incoming calll

:INFO:App Identifier: com.apple.mobilephone
:INFO:___________app_identifer = com.apple.mobilephone
:DEBUG:Attribute ID 1
:DEBUG:Attribute finished!
:INFO:+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
:INFO:attr_id = 1
:INFO:Title: 闄堥紟
:INFO:title length = 6
:INFO:
E9 99 88 E9 BC 8E                                闄堥.?
:INFO:___________title = 闄堥紟
:DEBUG:Attribute ID 2
:DEBUG:Attribute LEN 0
:INFO:+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
:INFO:attr_id = 2
:INFO:Subtitle: (N/A)
:DEBUG:Attribute ID 3
:DEBUG:Attribute finished!
:INFO:+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
:INFO:attr_id = 3
:INFO:Message: Incoming Call
:INFO:message length = 13
:INFO:
49 6E 63 6F 6D 69 6E 67 20 43 61 6C 6C           Incoming Call

---------------------------------------
为什么title有19个字节？？？
:INFO:App Identifier: com.apple.mobilephone
:INFO:___________app_identifer = com.apple.mobilephone
:DEBUG:Attribute ID 1
:DEBUG:Attribute finished!
:INFO:+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
:INFO:attr_id = 1
:INFO:Title: 鈥?51 7924 0648鈥
:INFO:title length = 19
:INFO:
E2 80 AD 31 35 31 20 37 39 32 34 20 30 36 34 38  ??51 7924 0648
E2 80 AC                                         ??
:INFO:___________title = 鈥?51 7924 0648鈥
:DEBUG:Attribute ID 2
:DEBUG:Attribute LEN 0

*********************************************************/

//BLE_ANCS_C_ARRAY_DEF(m_ancs_c_list, NRF_SDH_BLE_PERIPHERAL_LINK_COUNT);  
//要特别注意这里！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！1
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!需要深入理解Nordic如何对多个link进行调度
BLE_ANCS_C_ARRAY_DEF(m_ancs_c_list, NRF_SDH_BLE_TOTAL_LINK_COUNT);  

#define ATTR_DATA_SIZE			 	32
#define ATTR_DATA_SIZE_MESSAGE		BLE_ANCS_ATTR_DATA_MAX

static ble_ancs_c_evt_notif_t m_notification_latest[NRF_SDH_BLE_PERIPHERAL_LINK_COUNT];                          /**< Local copy to keep track of the newest arriving notifications. */
static ble_ancs_c_attr_t      m_notif_attr_latest[NRF_SDH_BLE_PERIPHERAL_LINK_COUNT];                            /**< Local copy of the newest notification attribute. */
static ble_ancs_c_attr_t      m_notif_attr_app_id_latest[NRF_SDH_BLE_PERIPHERAL_LINK_COUNT];                     /**< Local copy of the newest app attribute. */

static uint8_t m_attr_appid[NRF_SDH_BLE_PERIPHERAL_LINK_COUNT][ATTR_DATA_SIZE];                           /**< Buffer to store attribute data. */
static uint8_t m_attr_title[NRF_SDH_BLE_PERIPHERAL_LINK_COUNT][ATTR_DATA_SIZE];                           /**< Buffer to store attribute data. */
static uint8_t m_attr_subtitle[NRF_SDH_BLE_PERIPHERAL_LINK_COUNT][ATTR_DATA_SIZE];                        /**< Buffer to store attribute data. */
static uint8_t m_attr_message[NRF_SDH_BLE_PERIPHERAL_LINK_COUNT][ATTR_DATA_SIZE_MESSAGE];
static uint8_t m_attr_message_size[NRF_SDH_BLE_PERIPHERAL_LINK_COUNT][ATTR_DATA_SIZE];                    /**< Buffer to store attribute data. */
static uint8_t m_attr_date[NRF_SDH_BLE_PERIPHERAL_LINK_COUNT][ATTR_DATA_SIZE];                            /**< Buffer to store attribute data. */
static uint8_t m_attr_posaction[NRF_SDH_BLE_PERIPHERAL_LINK_COUNT][ATTR_DATA_SIZE];                       /**< Buffer to store attribute data. */
static uint8_t m_attr_negaction[NRF_SDH_BLE_PERIPHERAL_LINK_COUNT][ATTR_DATA_SIZE];                       /**< Buffer to store attribute data. */
static uint8_t m_attr_disp_name[NRF_SDH_BLE_PERIPHERAL_LINK_COUNT][ATTR_DATA_SIZE];     

static bool m_ancs_discovered[NRF_SDH_BLE_PERIPHERAL_LINK_COUNT]  = {false};                          /**< Bool to keep track of when both ancs and gatts have been disovered. Only then do we want to save the peer data. */
static bool m_gatts_discovered[NRF_SDH_BLE_PERIPHERAL_LINK_COUNT] = {false};                          /**< Bool to keep track of when both ancs and gatts have been disovered. Only then do we want to save the peer data. */

/**@brief String literals for the iOS notification categories. used then printing to UART. */
static const char * lit_catid[BLE_ANCS_NB_OF_CATEGORY_ID] =
{
    "Other",
    "Incoming Call",
    "Missed Call",
    "Voice Mail",
    "Social",
    "Schedule",
    "Email",
    "News",
    "Health And Fitness",
    "Business And Finance",
    "Location",
    "Entertainment"
};

/**@brief String literals for the iOS notification event types. Used then printing to UART. */
static const char * lit_eventid[BLE_ANCS_NB_OF_EVT_ID] =
{
    "Added",
    "Modified",
    "Removed"
};

/**@brief String literals for the iOS notification attribute types. Used when printing to UART. */
static const char * lit_attrid[BLE_ANCS_NB_OF_NOTIF_ATTR] =
{
    "App Identifier",
    "Title",
    "Subtitle",
    "Message",
    "Message Size",
    "Date",
    "Positive Action Label",
    "Negative Action Label"
};

/**@brief String literals for the iOS notification attribute types. Used When printing to UART. */
static const char * lit_appid[BLE_ANCS_NB_OF_APP_ATTR] =
{
    "Display Name"
};

void perform_positive_action(uint16_t conn_handle)
{
	uint32_t ret;
	NRF_LOG_INFO("Performing Positive Action.\r\n");
    APP_ERROR_CHECK(ret);
}

void perform_negative_action(uint16_t conn_handle)
{
	uint32_t ret;
	NRF_LOG_INFO("Performing Negative Action.\r\n");
    APP_ERROR_CHECK(ret);
}

void ancs_c_request_attribute(void * p_event_data, uint16_t event_size)
{
	NRF_LOG_INFO("FILE = %s, FUNC = %s, LINE = %d\r\n", (uint32_t)__FILE__, (uint32_t)__FUNCTION__, __LINE__);
	uint16_t conn_handle;
	conn_handle = (uint16_t)(*((uint32_t*)p_event_data));//Tommy Debug, Need modify
	nrf_ble_ancs_c_request_attrs(&m_ancs_c_list[conn_handle], &m_notification_latest[conn_handle]);
}

/**@brief Function for setting up GATTC notifications from the Notification Provider.
 *
 * @details This function is called when a successful connection has been established.
 */
//static void apple_notification_setup(void)
void apple_notification_setup(ble_ancs_c_t             * p_ancs)
{
    ret_code_t ret;

	NRF_LOG_INFO("-----------------apple_notification_setup\r\n");

	//Need modify when using rtos!
    nrf_delay_ms(100); // Delay because we cannot add a CCCD to close to starting encryption. iOS specific.

    ret = ble_ancs_c_notif_source_notif_enable(p_ancs);
    APP_ERROR_CHECK(ret);

    ret = ble_ancs_c_data_source_notif_enable(p_ancs);
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEBUG("Notifications Enabled.\r\n");
}

/**@brief Function for printing an iOS notification.
 *
 * @param[in] p_notif  Pointer to the iOS notification.
 */
void notif_print(ble_ancs_c_evt_notif_t * p_notif)
{
    NRF_LOG_INFO("\r\nNotification\r\n");
    NRF_LOG_INFO("Event:       %s\r\n", (uint32_t)lit_eventid[p_notif->evt_id]);
    NRF_LOG_INFO("Category ID: %s\r\n", (uint32_t)lit_catid[p_notif->category_id]);
	NRF_LOG_INFO("Category Cnt:%u\r\n", (unsigned int) p_notif->category_count);
    NRF_LOG_INFO("UID:         %u\r\n", (unsigned int) p_notif->notif_uid);

    NRF_LOG_INFO("Flags:\r\n");
    if(p_notif->evt_flags.silent == 1)
    {
        NRF_LOG_INFO(" Silent\r\n");
    }
    if(p_notif->evt_flags.important == 1)
    {
        NRF_LOG_INFO(" Important\r\n");
    }
    if(p_notif->evt_flags.pre_existing == 1)
    {
        NRF_LOG_INFO(" Pre-existing\r\n");
    }
    if(p_notif->evt_flags.positive_action == 1)
    {
        NRF_LOG_INFO(" Positive Action\r\n");
    }
    if(p_notif->evt_flags.negative_action == 1)
    {
        NRF_LOG_INFO(" Negative Action\r\n");
    }
}

/**@brief Function for printing iOS notification attribute data.
 *
 * @param[in] p_attr Pointer to an iOS notification attribute.
 */
static void notif_attr_print(ble_ancs_c_attr_t * p_attr)
{
	uint8_t match = false;
	push_notification_type_t push_notification_type;

	NRF_LOG_INFO("attr_id = %d\r\n", p_attr->attr_id);
	
    if (p_attr->attr_len != 0)
    {
        NRF_LOG_INFO("%s: %s\r\n", (uint32_t)lit_attrid[p_attr->attr_id], nrf_log_push((char *)p_attr->p_attr_data));
    }
    else if (p_attr->attr_len == 0)
    {
        NRF_LOG_INFO("%s: (N/A)\r\n", (uint32_t)lit_attrid[p_attr->attr_id]);
    }

	switch(p_attr->attr_id)
	{
		case BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER:
			break;
    	case BLE_ANCS_NOTIF_ATTR_ID_TITLE:
			break;
    	case BLE_ANCS_NOTIF_ATTR_ID_SUBTITLE:
			break;
    	case BLE_ANCS_NOTIF_ATTR_ID_MESSAGE:
			break;
    	case BLE_ANCS_NOTIF_ATTR_ID_MESSAGE_SIZE:
			break;
    	case BLE_ANCS_NOTIF_ATTR_ID_DATE:
			break;
    	case BLE_ANCS_NOTIF_ATTR_ID_POSITIVE_ACTION_LABEL:
			break;
    	case BLE_ANCS_NOTIF_ATTR_ID_NEGATIVE_ACTION_LABEL:
			break;
		default:
			break;
	}
	
}

/**@brief Function for printing iOS notification attribute data.
 *
 * @param[in] p_attr Pointer to an iOS App attribute.
 */
static void app_attr_print(ble_ancs_c_attr_t * p_attr)
{
    if (p_attr->attr_len != 0)
    {
        NRF_LOG_INFO("%s: %s\r\n", (uint32_t)lit_appid[p_attr->attr_id], (uint32_t)p_attr->p_attr_data);
    }
    else if (p_attr->attr_len == 0)
    {
        NRF_LOG_INFO("%s: (N/A)\r\n", (uint32_t) lit_appid[p_attr->attr_id]);
    }
}

/**@brief Function for printing out errors that originated from the Notification Provider (iOS).
 *
 * @param[in] err_code_np Error code received from NP.
 */
static void err_code_print(uint16_t err_code_np)
{
    switch (err_code_np)
    {
        case BLE_ANCS_NP_UNKNOWN_COMMAND:
            NRF_LOG_INFO("Error: Command ID was not recognized by the Notification Provider. \r\n");
            break;

        case BLE_ANCS_NP_INVALID_COMMAND:
            NRF_LOG_INFO("Error: Command failed to be parsed on the Notification Provider. \r\n");
            break;

        case BLE_ANCS_NP_INVALID_PARAMETER:
            NRF_LOG_INFO("Error: Parameter does not refer to an existing object on the Notification Provider. \r\n");
            break;

        case BLE_ANCS_NP_ACTION_FAILED:
            NRF_LOG_INFO("Error: Perform Notification Action Failed on the Notification Provider. \r\n");
            break;

        default:
            break;
    }
}

/**@brief Function for handling the Apple Notification Service client errors.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void apple_notification_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for handling the Apple Notification Service client.
 *
 * @details This function is called for all events in the Apple Notification client that
 *          are passed to the application.
 *
 * @param[in] p_evt  Event received from the Apple Notification Service client.
 */
static void ancs_c_evt_handler(ble_ancs_c_evt_t * p_evt)
{
	#if 1
    switch (p_evt->evt_type)
    {
        case BLE_ANCS_C_EVT_DISCOVERY_COMPLETE:
        {
            ret_code_t ret = NRF_SUCCESS;
            NRF_LOG_DEBUG("Apple Notification Center Service discovered on the server.");
            ret = nrf_ble_ancs_c_handles_assign(&m_ancs_c_list[p_evt->conn_handle], p_evt->conn_handle, &p_evt->service);
            APP_ERROR_CHECK(ret);

			#if 0
            pm_peer_id_t peer_id;
            ret = pm_peer_id_get(p_evt->conn_handle, &peer_id);
            APP_ERROR_CHECK(ret);

            // Copy the needed ANCS handles into a ble_gatt_db_srv_t struct that will be stored in
            // flash.
            ble_gatt_db_char_t * p_char = m_peer_srv_buf[1].charateristics;
            memset(&m_peer_srv_buf[1], 0, sizeof(m_peer_srv_buf[1]));

            p_char[0].characteristic = p_evt->service.control_point_char;
            p_char[1].characteristic = p_evt->service.notif_source_char;
            p_char[1].cccd_handle    = p_evt->service.notif_source_cccd.handle;
            p_char[2].characteristic = p_evt->service.data_source_char;
            p_char[2].cccd_handle    = p_evt->service.data_source_cccd.handle;

            m_ancs_discovered[p_evt->conn_handle] = true;

            if (m_gatts_discovered && m_ancs_discovered)
            {
                ret = pm_peer_data_remote_db_store(peer_id,
                                                   (ble_gatt_db_srv_t *)m_peer_srv_buf,
                                                   sizeof(m_peer_srv_buf),
                                                   NULL);
                if (ret == NRF_ERROR_STORAGE_FULL)
                {
                    ret = fds_gc();
                }
                APP_ERROR_CHECK(ret);
            }
			#endif
            apple_notification_setup(&m_ancs_c_list[p_evt->conn_handle]);
        } break;

        case BLE_ANCS_C_EVT_NOTIF:
            m_notification_latest[p_evt->conn_handle] = p_evt->notif;
            notif_print(&m_notification_latest[p_evt->conn_handle]);
            break;

        case BLE_ANCS_C_EVT_NOTIF_ATTRIBUTE:
            m_notif_attr_latest[p_evt->conn_handle] = p_evt->attr;
            notif_attr_print(&m_notif_attr_latest[p_evt->conn_handle]);
            if (p_evt->attr.attr_id == BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER)
            {
                m_notif_attr_app_id_latest[p_evt->conn_handle] = p_evt->attr;
            }
            break;
        case BLE_ANCS_C_EVT_DISCOVERY_FAILED:
            NRF_LOG_DEBUG("Apple Notification Center Service not discovered on the server.");
            break;

        case BLE_ANCS_C_EVT_APP_ATTRIBUTE:
            app_attr_print(&p_evt->attr);
            break;
        case BLE_ANCS_C_EVT_NP_ERROR:
            err_code_print(p_evt->err_code_np);
            break;
        default:
            // No implementation needed.
            break;
    }
	#endif
}

void ly_ble_ancs_c_disc_handler(ble_db_discovery_evt_t * p_evt)
{
	//感觉还是数据越界产生的问题，conn_handle和m_ancs_c_list数据大小不匹配，不能直接这样用吧！！！
	ble_ancs_c_on_db_disc_evt(&m_ancs_c_list[p_evt->conn_handle], p_evt);
}

void ly_ble_ancs_c_init(void)
{
	ble_ancs_c_init_t ancs_c_init;
	uint32_t ret;
	
	//for(uint8_t i = 0; i < NRF_SDH_BLE_PERIPHERAL_LINK_COUNT; i++)
	for(uint8_t i = 0; i < NRF_SDH_BLE_TOTAL_LINK_COUNT; i++)
	{
		// Init the Apple Notification Center Service client module.
    	memset(&ancs_c_init, 0, sizeof(ancs_c_init));

    	ret = nrf_ble_ancs_c_attr_add(&m_ancs_c_list[i],
                                  BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER,
                                  &m_attr_appid[i][0],
                                  ATTR_DATA_SIZE);
    	APP_ERROR_CHECK(ret);

    	ret = nrf_ble_ancs_c_app_attr_add(&m_ancs_c_list[i],
                                      BLE_ANCS_APP_ATTR_ID_DISPLAY_NAME,
                                      &m_attr_disp_name[i][0],
                                      ATTR_DATA_SIZE);
    	APP_ERROR_CHECK(ret);

    	ret = nrf_ble_ancs_c_attr_add(&m_ancs_c_list[i],
                                  BLE_ANCS_NOTIF_ATTR_ID_TITLE,
                                  &m_attr_title[i][0],
                                  ATTR_DATA_SIZE);
    	APP_ERROR_CHECK(ret);

    	ret = nrf_ble_ancs_c_attr_add(&m_ancs_c_list[i],
                                  BLE_ANCS_NOTIF_ATTR_ID_MESSAGE,
                                  &m_attr_message[i][0],
                                  ATTR_DATA_SIZE);
    	APP_ERROR_CHECK(ret);

   	 	ret = nrf_ble_ancs_c_attr_add(&m_ancs_c_list[i],
                                  BLE_ANCS_NOTIF_ATTR_ID_SUBTITLE,
                                  &m_attr_subtitle[i][0],
                                  ATTR_DATA_SIZE);
    	APP_ERROR_CHECK(ret);

    	ret = nrf_ble_ancs_c_attr_add(&m_ancs_c_list[i],
                                  BLE_ANCS_NOTIF_ATTR_ID_MESSAGE_SIZE,
                                  &m_attr_message_size[i][0],
                                  ATTR_DATA_SIZE);
    	APP_ERROR_CHECK(ret);

   	 	ret = nrf_ble_ancs_c_attr_add(&m_ancs_c_list[i],
                                  BLE_ANCS_NOTIF_ATTR_ID_DATE,
                                  &m_attr_date[i][0],
                                  ATTR_DATA_SIZE);
    	APP_ERROR_CHECK(ret);

    	ret = nrf_ble_ancs_c_attr_add(&m_ancs_c_list[i],
                                  BLE_ANCS_NOTIF_ATTR_ID_POSITIVE_ACTION_LABEL,
                                  &m_attr_posaction[i][0],
                                  ATTR_DATA_SIZE);
    	APP_ERROR_CHECK(ret);

    	ret = nrf_ble_ancs_c_attr_add(&m_ancs_c_list[i],
                                  BLE_ANCS_NOTIF_ATTR_ID_NEGATIVE_ACTION_LABEL,
                                  &m_attr_negaction[i][0],
                                  ATTR_DATA_SIZE);
    	APP_ERROR_CHECK(ret);

    	ancs_c_init.evt_handler   = ancs_c_evt_handler;
    	ancs_c_init.error_handler = apple_notification_error_handler;

    	ret = ble_ancs_c_init(&m_ancs_c_list[i], &ancs_c_init);
    	APP_ERROR_CHECK(ret);
	}
}

