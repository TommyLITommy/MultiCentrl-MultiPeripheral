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
#include "hardware."

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

extern void print_ble_db_discovery_info(void);

void sys_tick_timeout_handler(void *p_context)//Attention please, It is ok to add so many funciton in this handler??? 
{
	NRF_LOG_INFO("sys_tick:%d", sys_tick++);
	print_ble_db_discovery_info();
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


/**@brief Application main function.
 */
int main(void)
{
	uint32_t err_code;
    bool erase_bonds;

    log_init();
    
    timers_init();

	buttons_leds_init(&erase_bonds);
	
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
