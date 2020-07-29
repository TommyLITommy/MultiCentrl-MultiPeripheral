#include "app_util.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "ly_ble_c.h"
#include "ly_ble_tus_c.h"
#include "uart_protocol.h"
#include "uart_protocol_ble_central_handler.h"

extern void send_data_to_remote(uint8_t *p_data, uint16_t length);

void uart_protocol_group_id_ble_central_handler(uint8_t *p_data, uint16_t length)
{
	uint16_t payload_length;

	payload_length = uint16_decode(&p_data[UART_COMMAND_PAYLOAD_LENGTH_OFFSET]);

	switch(p_data[UART_COMMAND_COMMAND_ID_OFFSET])
    {
        case GID_BLE_C_CID_PTD_BLE_CENTRAL_MODE_SET:
        	{
        		
        	}
			break;
    	case GID_BLE_C_CID_PTD_SET_SCAN_PARAMETER:
    		{
    			//seldom been used!
    		}
			break;
    	case GID_BLE_C_CID_PTD_ACQUIRE_SCAN_PARAMETER:
			{
			
			}
			break;
    	case GID_BLE_C_CID_PTD_SCAN_START:
    		{
    			scan_start();
    		}
			break;
    	case GID_BLE_C_CID_PTD_SCAN_STOP:
    		{
    			scan_stop();
    		}
			break;
    	case GID_BLE_C_CID_PTD_CONNECT:
    		{
    			//mac_address
    		}
			break;
    	case GID_BLE_C_CID_PTD_DISCONNECT:
    		{
    			//mac_address or handle??
    		}
			break;
		case GID_BLE_C_CID_PTD_READ_CHAR:
    		{
    			
    		}
			break;
    	case GID_BLE_C_CID_PTD_WRITE_CHAR:
    		{
    			/*
    			payload format:handle + mac_address + data to be send
    			using handle and mac address to double check
				*/
				//payload_length check
    			uint16_t handle;
				uint16_t data_length;
				handle = uint16_decode(&p_data[UART_COMMAND_PAYLOAD_OFFSET]);
				data_length = payload_length - 2 - 6;

				NRF_LOG_INFO("handle:%d", handle);
				NRF_LOG_RAW_INFO("mac_address:");
				NRF_LOG_HEXDUMP_INFO(&p_data[UART_COMMAND_PAYLOAD_OFFSET + 2], 6);
				NRF_LOG_INFO("data_length:%d", data_length);
				NRF_LOG_INFO("Sequence:%d", p_data[UART_COMMAND_PAYLOAD_OFFSET + 2 + 6]);
				send_data_to_remote(&p_data[UART_COMMAND_PAYLOAD_OFFSET + 2 + 6], data_length);
				//NRF_LOG_HEXDUMP_INFO(&p_data[UART_COMMAND_PAYLOAD_OFFSET + 2 + 6], data_length);
				//get ble_tus_c instance by handle
				//judge whether it is connect or not?
				//ble_tus_c_t * p_ble_tus_c;
				//ble_tus_c_string_send(p_ble_tus_c, &p_data[UART_COMMAND_PAYLOAD_OFFSET + 2 + 6], data_length);

    		}
			break;
		default:
			break;
    }
}



