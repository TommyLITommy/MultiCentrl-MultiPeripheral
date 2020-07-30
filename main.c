/**
 * Copyright (c) 2014 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup ble_sdk_uart_over_ble_main main.c
 * @{
 * @ingroup  ble_sdk_app_nus_eval
 * @brief    UART over BLE application main file.
 *
 * This file contains the source code for a sample application that uses the Nordic UART service.
 * This application uses the @ref srvlib_conn_params module.
 */


#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "peer_manager.h"
#include "app_timer.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "bsp_btn_ble.h"
#include "nrf_pwr_mgmt.h"

#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "ly_ble.h"
#include "ly_ble_p.h"
#include "ly_ble_c.h"
#include "sys_info.h"
#include "sys_malloc.h"

APP_TIMER_DEF(sys_tick_timer_id);

uint32_t sys_tick;

/**@brief Function for initializing the timer module.
 */
static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}

typedef struct
{
    uint32_t offset;
    uint32_t size;
}icon_font_flash_info_t;

extern const icon_font_flash_info_t icon_font_flash_info[];

void sys_tick_timeout_handler(void *p_context)//Attention please, It is ok to add so many funciton in this handler??? 
{
	NRF_LOG_INFO("icon_font_flash_info[10]:%d,%d", icon_font_flash_info[10].offset, icon_font_flash_info[10].size);
	NRF_LOG_INFO("sys_tick:%d", sys_tick++);
	//print_ble_db_discovery_info();

	print_ly_ble_p();
	print_ly_ble_c();
}


bool do_database_discovery = false;

static void bsp_event_handler(bsp_event_t event)
{
    switch (event)
    {
        case BSP_EVENT_KEY_0:
			NRF_LOG_INFO("BSP_EVENT_KEY_0");
			do_database_discovery = true; 
			//ly_ble_db_discovery_start(lastest_conn_handle);
            break;

        case BSP_EVENT_KEY_1:
         	NRF_LOG_INFO("BSP_EVENT_KEY_1");
        	{
        		pm_conn_sec_status_t pm_conn_sec_status;
				pm_conn_sec_status_get(lastest_conn_handle, &pm_conn_sec_status);
				NRF_LOG_INFO("connected:%d, encrypted:%d, mitm_protected:%d, bonded:%d", pm_conn_sec_status.connected, pm_conn_sec_status.encrypted, pm_conn_sec_status.mitm_protected, pm_conn_sec_status.bonded);
        	}
            break;

        case BSP_EVENT_KEY_2:
            NRF_LOG_INFO("BSP_EVENT_KEY_2");
            break;

        case BSP_EVENT_KEY_3:
            NRF_LOG_INFO("BSP_EVENT_KEY_3");
            break;

        default:
            break;
    }
}

static void buttons_leds_init(bool * p_erase_bonds)
{
    uint32_t ret;
    bsp_event_t startup_event;

    ret = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(ret);

    ret = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(ret);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}

static void uart_rx_command_handler(uint8_t *p_data, uint16_t length)
{
	uint8_t *p_start_addr;
    
    NRF_LOG_INFO("Ready allocate %04x space\r\n", length);
	//NRF_LOG_HEXDUMP_INFO(p_data, length);
	//send_data_to_remote(p_data, length);
	sys_info.uart_protocol.uart_protocol_rx_handler(p_data, length);
	#if 0
	p_start_addr = sys_malloc_apply(length, MEMORY_USAGE_UART_RX, __LINE__);
    
	if(p_start_addr != NULL)
	{
        NRF_LOG_INFO("store data into queue\r\n");
        memcpy(p_start_addr, p_data, length);
		sys_queue_put(&sys_info.rx_queue, ELEMENT_TYPE_UART, p_start_addr, length);
	}
    else
    {
        NRF_LOG_INFO("p_start_addr == NULL\r\n");
    }
	#endif
}

typedef enum
{
	//ui_main
	ICON_ID_BLUETOOTH,
	ICON_ID_BATTERY_00,
	ICON_ID_BATTERY_01,
	ICON_ID_BATTERY_02,
	ICON_ID_BATTERY_03,
	ICON_ID_BATTERY_04,
	ICON_ID_HEART_RATE,
	ICON_ID_BLOOD_PRESSURE,
	ICON_ID_OVAL_FRAME_01,
	ICON_ID_OVAL_FRAME_02,
	//ui_analog_clock
	ICON_ID_ANALOG_CLOCK_DIAL,
	ICON_ID_ANALOG_CLOCK_HAND_HOUR,
	ICON_ID_ANALOG_CLOCK_HAND_MINUTE,
	//ui_heart_rate_measure
	ICON_ID_HEART_RATE_ICON,
	ICON_ID_HERAT_RATE_UNIT,
	//ui_blood_pressure_measure
	ICON_ID_BLOOD_PRESSURE_ICON,
	ICON_ID_BLOOD_PRESSURE_UNIT,
	//ui_notification
	ICON_ID_QQ,
	ICON_ID_WECHAT,
	ICON_ID_TEXT,
	ICON_ID_INCOMING_CALL,
	ICON_ID_OVAL_FRAME_03,
	ICON_ID_CIRCLE_01_BLUE,
	ICON_ID_CIRCLE_01_GRAY,
	//ui_charging
	ICON_ID_CHARGING_00,
	ICON_ID_CHARGING_01,
	ICON_ID_CHARGING_02,
	ICON_ID_CHARGING_03,
	ICON_ID_CHARGING_04,
	//ui_tire_setting
	ICON_ID_TIRE_SETTING_ICON,
	ICON_ID_TIRE_SETTING_NAME,
	//ui_ts_type_select
	ICON_ID_CIRCLE_02_BLUE,
	ICON_ID_CIRCLE_02_WHITE,
	/*tire select*/
	ICON_ID_TS_CIRCLE_BLUE,
	ICON_ID_TS_CIRCLE_DARK_GRAY,
	ICON_ID_TS_BACK_SELECT,
	ICON_ID_TS_BACK_DESELECT,
	ICON_ID_TS_OK_SELECT,
	ICON_ID_TS_OK_DESELECT,
	ICON_ID_TS_TIRE_BINDING_NAME,
	//ui_tp_motor
	ICON_ID_TP_MOTOR_BODY,
	ICON_ID_TP_MOTOR_TIRE_RED,
	ICON_ID_TP_MOTOR_TIRE_BLUE,
	ICON_ID_TP_MOTOR_TIRE_GRAY,
	//ui_tp_car ui_tp_suv
	ICON_ID_TP_CAR_BODY,
	ICON_ID_TP_SUV_BODY,
    ICON_ID_TP_CAR_SUV_H_TIRE_RED,
    ICON_ID_TP_CAR_SUV_H_TIRE_BLUE,
    ICON_ID_TP_CAR_SUV_H_TIRE_GRAY,
    ICON_ID_TP_CAR_SUV_V_TIRE_RED,
    ICON_ID_TP_CAR_SUV_V_TIRE_BLUE,
    ICON_ID_TP_CAR_SUV_V_TIRE_GRAY,
	ICON_ID_MAX,
}icon_id_t;

const icon_font_flash_info_t icon_font_flash_info[ICON_ID_MAX] = {
    { 0 , 640 },
    { 640 , 1200 },
    { 1840 , 1200 },
    { 3040 , 1200 },
    { 4240 , 1200 },
    { 5440 , 1200 },
    { 6640 , 960 },
    { 7600 , 1008 },
    { 8608 , 9200 },
    { 17808 , 9200 },
    { 4294967295 , 0 },
    { 4294967295 , 0 },
    { 4294967295 , 0 },
    { 27008 , 35912 },
    { 4294967295 , 0 },
    { 62920 , 35912 },
    { 4294967295 , 0 },
    { 98832 , 3200 },
    { 102032 , 3200 },
    { 105232 , 3200 },
    { 108432 , 7200 },
    { 115632 , 22000 },
    { 4294967295 , 0 },
    { 4294967295 , 0 },
    { 137632 , 37400 },
    { 175032 , 37400 },
    { 212432 , 37400 },
    { 249832 , 37400 },
    { 287232 , 37400 },
    { 324632 , 49928 },
    { 374560 , 3160 },
    { 377720 , 8192 },
    { 385912 , 8192 },
    { 394104 , 5000 },
    { 399104 , 5000 },
    { 404104 , 2592 },
    { 406696 , 2592 },
    { 409288 , 2592 },
    { 411880 , 2592 },
    { 414472 , 2556 },
    { 417028 , 40404 },
    { 4294967295 , 0 },
    { 4294967295 , 0 },
    { 4294967295 , 0 },
    { 457432 , 34560 },
    { 491992 , 34560 },
    { 526552 , 480 },
    { 527032 , 480 },
    { 527512 , 480 },
    { 527992 , 594 },
    { 528586 , 594 },
    { 529180 , 594 },
};


/**@brief Application main function.
 */
int main(void)
{
	uint32_t err_code;
    bool erase_bonds;

    log_init();
    
    timers_init();

	buttons_leds_init(&erase_bonds);

	#ifdef NRF52832_XXAA
	sys_malloc_init(&sys_info);
	sys_queue_init(&sys_info.rx_queue);
    
    sys_info_init(&sys_info);
    sys_info.hardware.drv_uart.drv_uart_rx_command_handler = uart_rx_command_handler;
	#endif
	
    power_management_init();

    ly_ble_init();

	scan_start();
	adv_start(erase_bonds);
    
    // Start execution.
    printf("\r\nUART started.\r\n");

	err_code = app_timer_create(&sys_tick_timer_id, APP_TIMER_MODE_REPEATED, sys_tick_timeout_handler);
	APP_ERROR_CHECK(err_code);

	err_code = app_timer_start(sys_tick_timer_id, APP_TIMER_TICKS(1000), NULL);
	APP_ERROR_CHECK(err_code);
	
    // Enter main loop.
    for (;;)
    {
    	if(do_database_discovery)
    	{
    		do_database_discovery = false;
    		ly_ble_db_discovery_start(lastest_conn_handle);
    	}
        idle_state_handle();
    }
}


/**
 * @}
 */


/*
Issue 2020-07-17
 	各个link之间的切换还存在一些问题，以及ancs_c，tus, tus_c 与conn_handle之间的关系！！！
 	很容易在不注意的情况下出现数组越界的情况，这个要特别特别注意
 	由于是多个link分时使用Radio，可能会出现busy的情况，这个要思考如何去解决！！！
 	在线调试时出现奇怪的现象导致异常，但是并没有触发APO_ERROR_CHECK, 此时暂停在线调试，光标停留
 	在hardfault异常，这里就要考虑可能是数据越界，导致对空指针的引用！！！
*/
