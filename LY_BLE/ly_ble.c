#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_bas.h"
#include "ble_hrs.h"
#include "ble_dis.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_freertos.h"
#include "app_timer.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "bsp_btn_ble.h"
#include "fds.h"
#include "ble_conn_state.h"
#include "ble_db_discovery.h"
#include "nrf_drv_clock.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "global_config.h"

#include "ly_ble_tus_c.h"
#include "ly_ble_ancs_c.h"

#include "ly_ble_c.h"
#include "ly_ble_p.h"
#include "ly_ble.h"

#define SEC_PARAM_BOND                      1                                       /**< Perform bonding. */
#define SEC_PARAM_MITM                      0                                       /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                      0                                       /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS                  0                                       /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES           BLE_GAP_IO_CAPS_NONE                    /**< No I/O capabilities. */
#define SEC_PARAM_OOB                       0                                       /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE              7                                       /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE              16                                      /**< Maximum encryption key size. */

#define DEAD_BEEF                           0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define OSTIMER_WAIT_FOR_QUEUE              2                                       /**< Number of ticks to wait for the timer queue to be ready */

#define VENDOR_SPECIFIC_UUID_COUNT      	8

#define CONN_CFG_TAG						1

BLE_DB_DISCOVERY_ARRAY_DEF(m_db_discovery, NRF_SDH_BLE_TOTAL_LINK_COUNT); 
NRF_BLE_GATT_DEF(m_gatt);                                           /**< GATT module instance. */

static ly_ble_c_t ly_ble_c;
static ly_ble_p_t ly_ble_p;

#define TOMMY_DEBUG_ENABLE_MULTI_BLE_CENTRAL 1

uint16_t lastest_conn_handle;

/**@brief Function for handling asserts in the SoftDevice.
 *
 * @details This function is called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and is not meant for the final product. You need to analyze
 *          how your product is supposed to react in case of assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num     Line number of the failing assert call.
 * @param[in] p_file_name  File name of the failing assert call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(0xDEADBEEF, line_num, p_file_name);
}

/**@brief Function for checking whether a bluetooth stack event is an advertising timeout.
 *
 * @param[in] p_ble_evt Bluetooth stack event.
 */
static bool ble_evt_is_advertising_timeout(ble_evt_t const * p_ble_evt)
{
    return (p_ble_evt->header.evt_id == BLE_GAP_EVT_ADV_SET_TERMINATED);
}

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    uint16_t conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
    uint16_t role        = ble_conn_state_role(conn_handle);

    // Based on the role this device plays in the connection, dispatch to the right handler.
    if (role == BLE_GAP_ROLE_PERIPH || ble_evt_is_advertising_timeout(p_ble_evt))
    {
    	#ifdef ENABLE_PAIRING
    	pm_handler_secure_on_connection(p_ble_evt);
		#endif
		ly_ble_p.on_ble_peripheral_evt(p_ble_evt);
    }
    else if ((role == BLE_GAP_ROLE_CENTRAL) || (p_ble_evt->header.evt_id == BLE_GAP_EVT_ADV_REPORT))
    {
    	#ifdef TOMMY_DEBUG_ENABLE_MULTI_BLE_CENTRAL
        ly_ble_c.on_ble_central_evt(p_ble_evt);
		#endif
    }
}


/**@brief Function for handling database discovery events.
 *
 * @details This function is a callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function forwards the events
 *          to their respective services.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    NRF_LOG_DEBUG("call to ble_lbs_on_db_disc_evt for instance %d and link 0x%x!",
                  p_evt->conn_handle,
                  p_evt->conn_handle);

	ly_ble_c_db_disc_handler(p_evt);
	ly_ble_p_db_disc_handler(p_evt);
}


/**
 * @brief Database discovery initialization.
 */
static void db_discovery_init(void)
{
    ret_code_t err_code = ble_db_discovery_init(db_disc_handler);
    APP_ERROR_CHECK(err_code);
}


static void ble_stack_init(void)//Tommy Debug, Pay more attention to the ram used by softdevice
{
	ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;

	err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    pm_handler_on_pm_evt(p_evt);
    pm_handler_disconnect_on_sec_failure(p_evt);//Tommy Debug, When security failed, it will disconnect the link!!!
    pm_handler_flash_clean(p_evt);

    switch (p_evt->evt_id)
    {
		case PM_EVT_BONDED_PEER_CONNECTED:           /**< @brief A connected peer has been identified as one with which we have a bond. When performing bonding with a peer for the first time, this event will not be sent until a new connection is established with the peer. When we are central, this event is always sent when the Peer Manager receives the @ref BLE_GAP_EVT_CONNECTED event. When we are peripheral, this event might in rare cases arrive later. */
			NRF_LOG_INFO("PM_EVT_BONDED_PEER_CONNECTED");
			//ly_ble_db_discovery_start(p_evt->conn_handle);
			break;
		case PM_EVT_CONN_SEC_START:                  /**< @brief A security procedure has started on a link, initiated either locally or remotely. The security procedure is using the last parameters provided via @ref pm_sec_params_set. This event is always followed by either a @ref PM_EVT_CONN_SEC_SUCCEEDED or a @ref PM_EVT_CONN_SEC_FAILED event. This is an informational event; no action is needed for the procedure to proceed. */
			NRF_LOG_INFO("PM_EVT_CONN_SEC_START");
			break;
		case PM_EVT_CONN_SEC_SUCCEEDED:              /**< @brief A link has been encrypted, either as a result of a call to @ref pm_conn_secure or a result of an action by the peer. The event structure contains more information about the circumstances. This event might contain a peer ID with the value @ref PM_PEER_ID_INVALID, which means that the peer (central) used an address that could not be identified, but it used an encryption key (LTK) that is present in the database. */
			NRF_LOG_INFO("PM_EVT_CONN_SEC_SUCCEEDED");
			//ly_ble_db_discovery_start(p_evt->conn_handle);
			break;
		case PM_EVT_CONN_SEC_FAILED:                 /**< @brief A pairing or encryption procedure has failed. In some cases, this means that security is not possible on this link (temporarily or permanently). How to handle this error depends on the application. */
			NRF_LOG_INFO("PM_EVT_CONN_SEC_FAILED");
			break;
		case PM_EVT_CONN_SEC_CONFIG_REQ:             /**< @brief The peer (central) has requested pairing, but a bond already exists with that peer. Reply by calling @ref pm_conn_sec_config_reply before the event handler returns. If no reply is sent, a default is used. */
			NRF_LOG_INFO("PM_EVT_CONN_SEC_CONFIG_REQ");
			{
				pm_conn_sec_config_t conn_sec_config = {.allow_repairing = true};
				pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
			}
			break;
		case PM_EVT_CONN_SEC_PARAMS_REQ:             /**< @brief Security parameters (@ref ble_gap_sec_params_t) are needed for an ongoing security procedure. Reply with @ref pm_conn_sec_params_reply before the event handler returns. If no reply is sent, the parameters given in @ref pm_sec_params_set are used. If a peripheral connection, the central's sec_params will be available in the event. */
			NRF_LOG_INFO("PM_EVT_CONN_SEC_PARAMS_REQ");
			break;
		case PM_EVT_STORAGE_FULL:                    /**< @brief There is no more room for peer data in flash storage. To solve this problem, delete data that is not needed anymore and run a garbage collection procedure in FDS. */
			NRF_LOG_INFO("PM_EVT_STORAGE_FULL");
			break;
		case PM_EVT_ERROR_UNEXPECTED:                /**< @brief An unrecoverable error happened inside Peer Manager. An operation failed with the provided error. */
			NRF_LOG_INFO("PM_EVT_ERROR_UNEXPECTED");
			break;
		case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:      /**< @brief A piece of peer data was stored, updated, or cleared in flash storage. This event is sent for all successful changes to peer data, also those initiated internally in Peer Manager. To identify an operation, compare the store token in the event with the store token received during the initiating function call. Events from internally initiated changes might have invalid store tokens. */
			NRF_LOG_INFO("PM_EVT_PEER_DATA_UPDATE_SUCCEEDED");
			break;
		case PM_EVT_PEER_DATA_UPDATE_FAILED:         /**< @brief A piece of peer data could not be stored, updated, or cleared in flash storage. This event is sent instead of @ref PM_EVT_PEER_DATA_UPDATE_SUCCEEDED for the failed operation. */
			NRF_LOG_INFO("PM_EVT_PEER_DATA_UPDATE_FAILED");
			break;
		case PM_EVT_PEER_DELETE_SUCCEEDED:           /**< @brief A peer was cleared from flash storage, for example because a call to @ref pm_peer_delete succeeded. This event can also be sent as part of a call to @ref pm_peers_delete or internal cleanup. */
			NRF_LOG_INFO("PM_EVT_PEER_DELETE_SUCCEEDED");
			break;
		case PM_EVT_PEER_DELETE_FAILED:              /**< @brief A peer could not be cleared from flash storage. This event is sent instead of @ref PM_EVT_PEER_DELETE_SUCCEEDED for the failed operation. */
			NRF_LOG_INFO("PM_EVT_PEER_DELETE_FAILED");
			break;
		case PM_EVT_PEERS_DELETE_SUCCEEDED:          /**< @brief A call to @ref pm_peers_delete has completed successfully. Flash storage now contains no peer data. */
			NRF_LOG_INFO("PM_EVT_PEERS_DELETE_SUCCEEDED");
			adv_start(false);
			break;
		case PM_EVT_PEERS_DELETE_FAILED:             /**< @brief A call to @ref pm_peers_delete has failed, which means that at least one of the peers could not be deleted. Other peers might have been deleted, or might still be queued to be deleted. No more @ref PM_EVT_PEERS_DELETE_SUCCEEDED or @ref PM_EVT_PEERS_DELETE_FAILED events are sent until the next time @ref pm_peers_delete is called. */
			NRF_LOG_INFO("PM_EVT_PEERS_DELETE_FAILED");
			break;
		case PM_EVT_LOCAL_DB_CACHE_APPLIED:          /**< @brief Local database values for a peer (taken from flash storage) have been provided to the SoftDevice. */
			NRF_LOG_INFO("PM_EVT_LOCAL_DB_CACHE_APPLIED");
			break;
		case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:     /**< @brief Local database values for a peer (taken from flash storage) were rejected by the SoftDevice, which means that either the database has changed or the user has manually set the local database to an invalid value (using @ref pm_peer_data_store). */
			NRF_LOG_INFO("PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED");
			break;
		case PM_EVT_SERVICE_CHANGED_IND_SENT:        /**< @brief A service changed indication has been sent to a peer, as a result of a call to @ref pm_local_database_has_changed. This event will be followed by a @ref PM_EVT_SERVICE_CHANGED_IND_CONFIRMED event if the peer acknowledges the indication. */
			NRF_LOG_INFO("PM_EVT_SERVICE_CHANGED_IND_SENT");
			break;
		case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:   /**< @brief A service changed indication that was sent has been confirmed by a peer. The peer can now be considered aware that the local database has changed. */
			NRF_LOG_INFO("PM_EVT_SERVICE_CHANGED_IND_CONFIRMED");
			break;
		case PM_EVT_SLAVE_SECURITY_REQ:              /**< @brief The peer (peripheral) has requested link encryption, which has been enabled. */
			NRF_LOG_INFO("PM_EVT_SLAVE_SECURITY_REQ");
			break;
		case PM_EVT_FLASH_GARBAGE_COLLECTED:         /**< @brief The flash has been garbage collected (By FDS), possibly freeing up space. */
			NRF_LOG_INFO("PM_EVT_FLASH_GARBAGE_COLLECTED");
			break;
		case PM_EVT_FLASH_GARBAGE_COLLECTION_FAILED: /**< @brief Garbage collection was attempted but failed. */
			NRF_LOG_INFO("PM_EVT_FLASH_GARBAGE_COLLECTION_FAILED");
			break;
        default:
            break;
    }
}

/**@brief Function for initializing the Peer Manager.
 */
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}

/*
<info> app: ly_ble_db_discovery_start

<info> app: addr of m_db_discovery[2]:0x200065C0

<info> app: sizeof(m_db_discovery[conn_handle]):716

<info> ble_db_disc: 02 addr of m_initialized = 0x2000669C

<info> ble_db_disc: ble_db_discovery_start,m_initialized:1

<info> ble_db_disc: m_num_of_handlers_reg:2

<info> app: sys_tick:37

<info> ble_db_disc: 01 addr of m_initialized = 0x2000669C

<info> ble_db_disc: m_initialized = 1, m_num_of_handlers_reg = 2

<info> app: sys_tick:38

<info> ble_db_disc: 01 addr of m_initialized = 0x2000669C

使用memset时要特别注意，这里出现了内存覆盖的问题！！！
这里是数组越界了，使用数组和memset时要特别注意！！！
这两天都是在解决这个堆栈溢出的问题，是个深刻的教训，在处理数组时，一定要特别注意下标不要越界！！！2020-07-16
*/

void ly_ble_db_discovery_start(uint16_t conn_handle)
{
	NRF_LOG_INFO("NRF_SDH_BLE_TOTAL_LINK_COUNT:%d", NRF_SDH_BLE_TOTAL_LINK_COUNT);
	NRF_LOG_INFO("ly_ble_db_discovery_start");
	#if 1
	uint32_t err_code;
	NRF_LOG_INFO("addr of m_db_discovery[%d]:0x%08x", conn_handle, &m_db_discovery[conn_handle]);
	NRF_LOG_INFO("sizeof(m_db_discovery[%d]):%d", conn_handle, sizeof(m_db_discovery[conn_handle]));
	memset(&m_db_discovery[conn_handle], 0, sizeof(m_db_discovery[conn_handle]));
	err_code = ble_db_discovery_start(&m_db_discovery[conn_handle],
											  conn_handle);

	if(err_code != NRF_ERROR_BUSY)
	{
		APP_ERROR_CHECK(err_code);
	}
	#endif
}

void ly_ble_init(void)
{
	ble_stack_init();
	peer_manager_init();
	db_discovery_init();
	gatt_init();
	#ifdef TOMMY_DEBUG_ENABLE_MULTI_BLE_CENTRAL
	ly_ble_c_init(&ly_ble_c);
	#endif
	ly_ble_p_init(&ly_ble_p);
}
