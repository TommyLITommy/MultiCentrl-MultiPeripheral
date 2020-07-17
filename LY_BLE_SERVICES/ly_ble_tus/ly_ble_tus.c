#include "sdk_common.h"
//#if NRF_MODULE_ENABLED(BLE_TUS)
#if 1
#include "ble.h"
#include "ly_ble_tus.h"
#include "ble_srv_common.h"

#define NRF_LOG_MODULE_NAME ble_tus
#if BLE_NUS_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       BLE_TUS_CONFIG_LOG_LEVEL
#define NRF_LOG_INFO_COLOR  BLE_TUS_CONFIG_INFO_COLOR
#define NRF_LOG_DEBUG_COLOR BLE_TUS_CONFIG_DEBUG_COLOR
#else // BLE_NUS_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       1
#endif // BLE_NUS_CONFIG_LOG_ENABLED
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();


#define BLE_UUID_TUS_TX_CHARACTERISTIC 0x0003               /**< The UUID of the TX Characteristic. */
#define BLE_UUID_TUS_RX_CHARACTERISTIC 0x0002               /**< The UUID of the RX Characteristic. */

#define BLE_TUS_MAX_RX_CHAR_LEN        BLE_TUS_MAX_DATA_LEN /**< Maximum length of the RX Characteristic (in bytes). */
#define BLE_TUS_MAX_TX_CHAR_LEN        BLE_TUS_MAX_DATA_LEN /**< Maximum length of the TX Characteristic (in bytes). */

#define TUS_BASE_UUID                  {{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x00, 0x00, 0x40, 0x6E}} /**< Used vendor specific UUID. */


/**@brief Function for handling the @ref BLE_GAP_EVT_CONNECTED event from the SoftDevice.
 *
 * @param[in] p_tus     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_connect(ble_tus_t * p_tus, ble_evt_t const * p_ble_evt)
{
    ret_code_t                 err_code;
    ble_tus_evt_t              evt;
    ble_gatts_value_t          gatts_val;
    uint8_t                    cccd_value[2];
    ble_tus_client_context_t * p_client = NULL;

	NRF_LOG_INFO("addr of p_tus:0x%08x", p_tus);

    err_code = blcm_link_ctx_get(p_tus->p_link_ctx_storage,
                                 p_ble_evt->evt.gap_evt.conn_handle,
                                 (void *) &p_client);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.",
                      p_ble_evt->evt.gap_evt.conn_handle);
    }

    /* Check the hosts CCCD value to inform of readiness to send data using the RX characteristic */
    memset(&gatts_val, 0, sizeof(ble_gatts_value_t));
    gatts_val.p_value = cccd_value;
    gatts_val.len     = sizeof(cccd_value);
    gatts_val.offset  = 0;

    err_code = sd_ble_gatts_value_get(p_ble_evt->evt.gap_evt.conn_handle,
                                      p_tus->rx_handles.cccd_handle,
                                      &gatts_val);

    if ((err_code == NRF_SUCCESS)     &&
        (p_tus->data_handler != NULL) &&
        ble_srv_is_notification_enabled(gatts_val.p_value))
    {
        if (p_client != NULL)
        {
            p_client->is_notification_enabled = true;
        }

        memset(&evt, 0, sizeof(ble_tus_evt_t));
        evt.type        = BLE_TUS_EVT_COMM_STARTED;
        evt.p_tus       = p_tus;
        evt.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        p_tus->data_handler(&evt);
    }
}


/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the SoftDevice.
 *
 * @param[in] p_tus     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_write(ble_tus_t * p_tus, ble_evt_t const * p_ble_evt)
{
    ret_code_t                    err_code;
    ble_tus_evt_t                 evt;
    ble_tus_client_context_t    * p_client;
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    err_code = blcm_link_ctx_get(p_tus->p_link_ctx_storage,
                                 p_ble_evt->evt.gatts_evt.conn_handle,
                                 (void *) &p_client);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.",
                      p_ble_evt->evt.gatts_evt.conn_handle);
    }

    memset(&evt, 0, sizeof(ble_tus_evt_t));
    evt.p_tus       = p_tus;
    evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
    evt.p_link_ctx  = p_client;

    if ((p_evt_write->handle == p_tus->tx_handles.cccd_handle) &&
        (p_evt_write->len == 2))
    {
        if (p_client != NULL)
        {
            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
            	NRF_LOG_INFO("notification enabled");
                p_client->is_notification_enabled = true;
                evt.type                          = BLE_TUS_EVT_COMM_STARTED;
            }
            else
            {
            	NRF_LOG_INFO("notification disabled");
                p_client->is_notification_enabled = false;
                evt.type                          = BLE_TUS_EVT_COMM_STOPPED;
            }

            if (p_tus->data_handler != NULL)
            {
                p_tus->data_handler(&evt);
            }

        }
    }
    else if ((p_evt_write->handle == p_tus->rx_handles.value_handle) &&
             (p_tus->data_handler != NULL))
    {
        evt.type                  = BLE_TUS_EVT_RX_DATA;
        evt.params.rx_data.p_data = p_evt_write->data;
        evt.params.rx_data.length = p_evt_write->len;

        p_tus->data_handler(&evt);
    }
    else
    {
        // Do Nothing. This event is not relevant for this service.
    }
}


/**@brief Function for handling the @ref BLE_GATTS_EVT_HVN_TX_COMPLETE event from the SoftDevice.
 *
 * @param[in] p_tus     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_hvx_tx_complete(ble_tus_t * p_tus, ble_evt_t const * p_ble_evt)
{
    ret_code_t                 err_code;
    ble_tus_evt_t              evt;
    ble_tus_client_context_t * p_client;

    err_code = blcm_link_ctx_get(p_tus->p_link_ctx_storage,
                                 p_ble_evt->evt.gatts_evt.conn_handle,
                                 (void *) &p_client);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.",
                      p_ble_evt->evt.gatts_evt.conn_handle);
        return;
    }

    if (p_client->is_notification_enabled)
    {
        memset(&evt, 0, sizeof(ble_tus_evt_t));
        evt.type        = BLE_TUS_EVT_TX_RDY;
        evt.p_tus       = p_tus;
        evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        p_tus->data_handler(&evt);
    }
}


void ble_tus_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)//Tommy Debug, p_context???
{
    if ((p_context == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    ble_tus_t * p_tus = (ble_tus_t *)p_context;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_tus, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_tus, p_ble_evt);
            break;

        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            on_hvx_tx_complete(p_tus, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}


uint32_t ble_tus_init(ble_tus_t * p_tus, ble_tus_init_t const * p_tus_init)
{
    ret_code_t            err_code;
    ble_uuid_t            ble_uuid;
    ble_uuid128_t         tus_base_uuid = TUS_BASE_UUID;
    ble_add_char_params_t add_char_params;

    VERIFY_PARAM_NOT_NULL(p_tus);
    VERIFY_PARAM_NOT_NULL(p_tus_init);

    // Initialize the service structure.
    p_tus->data_handler = p_tus_init->data_handler;

    /**@snippet [Adding proprietary Service to the SoftDevice] */
    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&tus_base_uuid, &p_tus->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_tus->uuid_type;
    ble_uuid.uuid = BLE_UUID_TUS_SERVICE;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_tus->service_handle);
    /**@snippet [Adding proprietary Service to the SoftDevice] */
    VERIFY_SUCCESS(err_code);

    // Add the RX Characteristic.
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid                     = BLE_UUID_TUS_RX_CHARACTERISTIC;
    add_char_params.uuid_type                = p_tus->uuid_type;
    add_char_params.max_len                  = BLE_TUS_MAX_RX_CHAR_LEN;
    add_char_params.init_len                 = sizeof(uint8_t);
    add_char_params.is_var_len               = true;
    add_char_params.char_props.write         = 1;
    add_char_params.char_props.write_wo_resp = 1;

    add_char_params.read_access  = SEC_OPEN;
    add_char_params.write_access = SEC_OPEN;    

    err_code = characteristic_add(p_tus->service_handle, &add_char_params, &p_tus->rx_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add the TX Characteristic.
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid              = BLE_UUID_TUS_TX_CHARACTERISTIC;
    add_char_params.uuid_type         = p_tus->uuid_type;
    add_char_params.max_len           = BLE_TUS_MAX_TX_CHAR_LEN;
    add_char_params.init_len          = sizeof(uint8_t);
    add_char_params.is_var_len        = true;
    add_char_params.char_props.notify = 1;

    add_char_params.read_access       = SEC_OPEN;
    add_char_params.write_access      = SEC_OPEN;
    add_char_params.cccd_write_access = SEC_OPEN;

    return characteristic_add(p_tus->service_handle, &add_char_params, &p_tus->tx_handles);
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
}


uint32_t ble_tus_data_send(ble_tus_t * p_tus,
                           uint8_t   * p_data,
                           uint16_t  * p_length,
                           uint16_t    conn_handle)
{
    ret_code_t                 err_code;
    ble_gatts_hvx_params_t     hvx_params;
    ble_tus_client_context_t * p_client;

    VERIFY_PARAM_NOT_NULL(p_tus);

	#if 0//Temporary comment out this code snippet, pay more attention to the link context!!!
    err_code = blcm_link_ctx_get(p_tus->p_link_ctx_storage, conn_handle, (void *) &p_client);
    VERIFY_SUCCESS(err_code);
	
    if ((conn_handle == BLE_CONN_HANDLE_INVALID) || (p_client == NULL))
    {
        return NRF_ERROR_NOT_FOUND;
    }

    if (!p_client->is_notification_enabled)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (*p_length > BLE_TUS_MAX_DATA_LEN)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
	#endif
	
    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_tus->tx_handles.value_handle;
    hvx_params.p_data = p_data;
    hvx_params.p_len  = p_length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(conn_handle, &hvx_params);
}

#endif // NRF_MODULE_ENABLED(BLE_NUS)

