#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "nrf_error.h"
#include "sys_info.h"

sys_info_t sys_info;

void sys_info_init(sys_info_t *p_sys_info)
{
    uart_protocol_init(&p_sys_info->uart_protocol);
	hardware_init(&p_sys_info->hardware);
}
