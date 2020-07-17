#ifndef __LY_BLE_TUS_C_H__
#define __LY_BLE_TUS_C_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_gatt.h"
#include "ble_db_discovery.h"
#include "nrf_sdh_ble.h"

#include "sdk_config.h"

#define BLE_TUS_C_BLE_OBSERVER_PRIO 2

#ifdef __cplusplus
extern "C" {
#endif

/*
tus represents "tommy uart service"
*/

/**@brief   Macro for defining a ble_tus_c instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_TUS_C_DEF(_name)                                                                        \
static ble_tus_c_t _name;                                                                           \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_TUS_C_BLE_OBSERVER_PRIO,                                                   \
                     ble_tus_c_on_ble_evt, &_name)

/**@brief   Macro for defining multiple ble_tus_c instances.
 *
 * @param   _name   Name of the array of instances.
 * @param   _cnt    Number of instances to define.
 */
#define BLE_TUS_C_ARRAY_DEF(_name, _cnt)                                                            \
static ble_tus_c_t _name[_cnt];                                                                     \
NRF_SDH_BLE_OBSERVERS(_name ## _obs,                                                                \
                      BLE_TUS_C_BLE_OBSERVER_PRIO,                                                  \
                      ble_tus_c_on_ble_evt, &_name, _cnt)


#define TUS_BASE_UUID                   {{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x00, 0x00, 0x40, 0x6E}} /**< Used vendor specific UUID. */

#define BLE_UUID_TUS_SERVICE            0x0001                      /**< The UUID of the Nordic UART Service. */
#define BLE_UUID_TUS_RX_CHARACTERISTIC  0x0002                      /**< The UUID of the RX Characteristic. */
#define BLE_UUID_TUS_TX_CHARACTERISTIC  0x0003                      /**< The UUID of the TX Characteristic. */

#define OPCODE_LENGTH 1
#define HANDLE_LENGTH 2

/**@brief   Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
#if defined(NRF_SDH_BLE_GATT_MAX_MTU_SIZE) && (NRF_SDH_BLE_GATT_MAX_MTU_SIZE != 0)
    #define BLE_TUS_MAX_DATA_LEN (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - OPCODE_LENGTH - HANDLE_LENGTH)
#else
    #define BLE_TUS_MAX_DATA_LEN (BLE_GATT_MTU_SIZE_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH)
    #warning NRF_SDH_BLE_GATT_MAX_MTU_SIZE is not defined.
#endif

/**@brief TUS Client event type. */
typedef enum
{
    BLE_TUS_C_EVT_DISCOVERY_COMPLETE,   /**< Event indicating that the TUS service and its characteristics was found. */
    BLE_TUS_C_EVT_TUS_TX_EVT,           /**< Event indicating that the central has received something from a peer. */
    BLE_TUS_C_EVT_DISCONNECTED          /**< Event indicating that the TUS server has disconnected. */
} ble_tus_c_evt_type_t;

/**@brief Handles on the connected peer device needed to interact with it. */
typedef struct
{
    uint16_t tus_tx_handle;      /**< Handle of the TUS TX characteristic as provided by a discovery. */
    uint16_t tus_tx_cccd_handle; /**< Handle of the CCCD of the TUS TX characteristic as provided by a discovery. */
    uint16_t tus_rx_handle;      /**< Handle of the TUS RX characteristic as provided by a discovery. */
} ble_tus_c_handles_t;

/**@brief Structure containing the NUS event data received from the peer. */
typedef struct
{
    ble_tus_c_evt_type_t evt_type;
    uint16_t             conn_handle;
    uint16_t             max_data_len;
    uint8_t            * p_data;
    uint16_t             data_len;
    ble_tus_c_handles_t  handles;     /**< Handles on which the Nordic Uart service characteristics was discovered on the peer device. This will be filled if the evt_type is @ref BLE_NUS_C_EVT_DISCOVERY_COMPLETE.*/
} ble_tus_c_evt_t;

// Forward declaration of the ble_tus_t type.
typedef struct ble_tus_c_s ble_tus_c_t;

/**@brief   Event handler type.
 *
 * @details This is the type of the event handler that should be provided by the application
 *          of this module to receive events.
 */
typedef void (*ble_tus_c_evt_handler_t)(ble_tus_c_t * p_ble_tus_c, ble_tus_c_evt_t const * p_evt);


/**@brief TUS Client structure. */
struct ble_tus_c_s
{
    uint8_t                 uuid_type;      /**< UUID type. */
    uint16_t                conn_handle;    /**< Handle of the current connection. Set with @ref ble_nus_c_handles_assign when connected. */
    ble_tus_c_handles_t     handles;        /**< Handles on the connected peer device needed to interact with it. */
    ble_tus_c_evt_handler_t evt_handler;    /**< Application event handler to be called when there is an event related to the TUS. */
};

/**@brief TUS Client initialization structure. */
typedef struct
{
    ble_tus_c_evt_handler_t evt_handler;
} ble_tus_c_init_t;

uint32_t ble_tus_c_init(ble_tus_c_t * p_ble_tus_c, ble_tus_c_init_t * p_ble_tus_c_init);
void ble_tus_c_on_db_disc_evt(ble_tus_c_t * p_ble_tus_c, ble_db_discovery_evt_t * p_evt);
void ble_tus_c_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);
uint32_t ble_tus_c_tx_notif_enable(ble_tus_c_t * p_ble_tus_c);
uint32_t ble_tus_c_string_send(ble_tus_c_t * p_ble_nus_c, uint8_t * p_string, uint16_t length);
uint32_t ble_tus_c_handles_assign(ble_tus_c_t *               p_ble_tus_c,
                                  uint16_t                    conn_handle,
                                  ble_tus_c_handles_t const * p_peer_handles);
#ifdef __cplusplus
}
#endif

#endif
