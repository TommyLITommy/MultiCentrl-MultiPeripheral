#include "sdk_common.h"
//#if NRF_MODULE_ENABLED(BLE_TUS_C)
#if 1
#include <stdlib.h>

#include "ble.h"
#include "ly_ble_tus_c.h"
#include "ble_gattc.h"
#include "ble_srv_common.h"
#include "app_error.h"

#define NRF_LOG_MODULE_NAME ble_tus_c
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

void ble_tus_c_on_db_disc_evt(ble_tus_c_t * p_ble_tus_c, ble_db_discovery_evt_t * p_evt)
{
    ble_tus_c_evt_t tus_c_evt;
    memset(&tus_c_evt,0,sizeof(ble_tus_c_evt_t));

    ble_gatt_db_char_t * p_chars = p_evt->params.discovered_db.charateristics;

    // Check if the TUS was discovered.
    if (    (p_evt->evt_type == BLE_DB_DISCOVERY_COMPLETE)
        &&  (p_evt->params.discovered_db.srv_uuid.uuid == BLE_UUID_TUS_SERVICE)
        &&  (p_evt->params.discovered_db.srv_uuid.type == p_ble_tus_c->uuid_type))
    {
        for (uint32_t i = 0; i < p_evt->params.discovered_db.char_count; i++)
        {
            switch (p_chars[i].characteristic.uuid.uuid)
            {
                case BLE_UUID_TUS_RX_CHARACTERISTIC:
                    tus_c_evt.handles.tus_rx_handle = p_chars[i].characteristic.handle_value;
                    break;

                case BLE_UUID_TUS_TX_CHARACTERISTIC:
                    tus_c_evt.handles.tus_tx_handle = p_chars[i].characteristic.handle_value;
                    tus_c_evt.handles.tus_tx_cccd_handle = p_chars[i].cccd_handle;
                    break;

                default:
                    break;
            }
        }
        if (p_ble_tus_c->evt_handler != NULL)
        {
            tus_c_evt.conn_handle = p_evt->conn_handle;
            tus_c_evt.evt_type    = BLE_TUS_C_EVT_DISCOVERY_COMPLETE;
            p_ble_tus_c->evt_handler(p_ble_tus_c, &tus_c_evt);
        }
    }
	else
	{
		//Do not found tus service!
	}
}

/**@brief     Function for handling Handle Value Notification received from the SoftDevice.
 *
 * @details   This function will uses the Handle Value Notification received from the SoftDevice
 *            and checks if it is a notification of the TUS TX characteristic from the peer. If
 *            it is, this function will decode the data and send it to the
 *            application.
 *
 * @param[in] p_ble_tus_c Pointer to the TUS Client structure.
 * @param[in] p_ble_evt   Pointer to the BLE event received.
 */
static void on_hvx(ble_tus_c_t * p_ble_tus_c, ble_evt_t const * p_ble_evt)
{
    // HVX can only occur from client sending.
    if (   (p_ble_tus_c->handles.tus_tx_handle != BLE_GATT_HANDLE_INVALID)
        && (p_ble_evt->evt.gattc_evt.params.hvx.handle == p_ble_tus_c->handles.tus_tx_handle)
        && (p_ble_tus_c->evt_handler != NULL))
    {
        ble_tus_c_evt_t ble_tus_c_evt;

        ble_tus_c_evt.evt_type = BLE_TUS_C_EVT_TUS_TX_EVT;
        ble_tus_c_evt.p_data   = (uint8_t *)p_ble_evt->evt.gattc_evt.params.hvx.data;
        ble_tus_c_evt.data_len = p_ble_evt->evt.gattc_evt.params.hvx.len;

        p_ble_tus_c->evt_handler(p_ble_tus_c, &ble_tus_c_evt);
        NRF_LOG_DEBUG("Client sending data.");
    }
}

uint32_t ble_tus_c_init(ble_tus_c_t * p_ble_tus_c, ble_tus_c_init_t * p_ble_tus_c_init)
{
    uint32_t      err_code;
    ble_uuid_t    uart_uuid;
    ble_uuid128_t tus_base_uuid = TUS_BASE_UUID;

    VERIFY_PARAM_NOT_NULL(p_ble_tus_c);
    VERIFY_PARAM_NOT_NULL(p_ble_tus_c_init);

	NRF_LOG_INFO("01_p_ble_tus_c->uuid_type:%d", p_ble_tus_c->uuid_type);

    err_code = sd_ble_uuid_vs_add(&tus_base_uuid, &p_ble_tus_c->uuid_type);
    VERIFY_SUCCESS(err_code);

	NRF_LOG_INFO("02_p_ble_tus_c->uuid_type:%d", p_ble_tus_c->uuid_type);
	
    uart_uuid.type = p_ble_tus_c->uuid_type;
    uart_uuid.uuid = BLE_UUID_TUS_SERVICE;

    p_ble_tus_c->conn_handle           = BLE_CONN_HANDLE_INVALID;
    p_ble_tus_c->evt_handler           = p_ble_tus_c_init->evt_handler;
    p_ble_tus_c->handles.tus_tx_handle = BLE_GATT_HANDLE_INVALID;
    p_ble_tus_c->handles.tus_rx_handle = BLE_GATT_HANDLE_INVALID;

    return ble_db_discovery_evt_register(&uart_uuid);//What is effect of calling this function
}

void ble_tus_c_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_tus_c_t * p_ble_tus_c = (ble_tus_c_t *)p_context;

    if ((p_ble_tus_c == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    if ( (p_ble_tus_c->conn_handle != BLE_CONN_HANDLE_INVALID)
       &&(p_ble_tus_c->conn_handle != p_ble_evt->evt.gap_evt.conn_handle)
       )
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GATTC_EVT_HVX:
            on_hvx(p_ble_tus_c, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            if (p_ble_evt->evt.gap_evt.conn_handle == p_ble_tus_c->conn_handle
                    && p_ble_tus_c->evt_handler != NULL)
            {
                ble_tus_c_evt_t tus_c_evt;

                tus_c_evt.evt_type = BLE_TUS_C_EVT_DISCONNECTED;

                p_ble_tus_c->conn_handle = BLE_CONN_HANDLE_INVALID;
                p_ble_tus_c->evt_handler(p_ble_tus_c, &tus_c_evt);
            }
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for creating a message for writing to the CCCD. */
static uint32_t cccd_configure(uint16_t conn_handle, uint16_t cccd_handle, bool enable)
{
    uint8_t buf[BLE_CCCD_VALUE_LEN];

    buf[0] = enable ? BLE_GATT_HVX_NOTIFICATION : 0;
    buf[1] = 0;

    ble_gattc_write_params_t const write_params =
    {
        .write_op = BLE_GATT_OP_WRITE_REQ,
        .flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE,
        .handle   = cccd_handle,
        .offset   = 0,
        .len      = sizeof(buf),
        .p_value  = buf
    };

    return sd_ble_gattc_write(conn_handle, &write_params);
}


uint32_t ble_tus_c_tx_notif_enable(ble_tus_c_t * p_ble_tus_c)
{
	NRF_LOG_INFO("ble_tus_c_tx_notif_enable, conn_handle:%d, tux_tx_cccd_handle:%d", p_ble_tus_c->conn_handle, p_ble_tus_c->handles.tus_tx_cccd_handle);
    VERIFY_PARAM_NOT_NULL(p_ble_tus_c);

    if ( (p_ble_tus_c->conn_handle == BLE_CONN_HANDLE_INVALID)
       ||(p_ble_tus_c->handles.tus_tx_cccd_handle == BLE_GATT_HANDLE_INVALID)
       )
    {
        return NRF_ERROR_INVALID_STATE;
    }
    return cccd_configure(p_ble_tus_c->conn_handle,p_ble_tus_c->handles.tus_tx_cccd_handle, true);
}


uint32_t ble_tus_c_string_send(ble_tus_c_t * p_ble_tus_c, uint8_t * p_string, uint16_t length)
{
    VERIFY_PARAM_NOT_NULL(p_ble_tus_c);

    if (length > BLE_TUS_MAX_DATA_LEN)
    {
        NRF_LOG_WARNING("Content too long.");
        return NRF_ERROR_INVALID_PARAM;
    }
    if (p_ble_tus_c->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        NRF_LOG_WARNING("Connection handle invalid.");
        return NRF_ERROR_INVALID_STATE;
    }

    ble_gattc_write_params_t const write_params =
    {
        .write_op = BLE_GATT_OP_WRITE_CMD,
        .flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE,
        .handle   = p_ble_tus_c->handles.tus_rx_handle,
        .offset   = 0,
        .len      = length,
        .p_value  = p_string//There exists a problem? Does the low layer copy the data ???
    };

    return sd_ble_gattc_write(p_ble_tus_c->conn_handle, &write_params);
}


uint32_t ble_tus_c_handles_assign(ble_tus_c_t               * p_ble_tus,
                                  uint16_t                    conn_handle,
                                  ble_tus_c_handles_t const * p_peer_handles)
{
    VERIFY_PARAM_NOT_NULL(p_ble_tus);

    p_ble_tus->conn_handle = conn_handle;
    if (p_peer_handles != NULL)
    {
        p_ble_tus->handles.tus_tx_cccd_handle = p_peer_handles->tus_tx_cccd_handle;
        p_ble_tus->handles.tus_tx_handle      = p_peer_handles->tus_tx_handle;
        p_ble_tus->handles.tus_rx_handle      = p_peer_handles->tus_rx_handle;
    }
    return NRF_SUCCESS;
}
#endif // NRF_MODULE_ENABLED(BLE_TUS_C)
