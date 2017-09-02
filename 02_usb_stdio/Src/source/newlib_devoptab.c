/*
 * newlib_write_read.c
 *
 *  Created on: 26.08.2017
 *      Author: Kamil Cukrowski
 */
#include "newlib_devoptab.h"

#include "usbd_cdc_if_ex.h"

/* reent.h implementation ------------------------------------------------------------- */

#define stdin_read_r     USB_CDC_read_r
#define stdout_write_r   USB_CDC_write_r
#define stderr_write_r   USB_CDC_write_r

const devoptab_t devoptab_list[] = {
	{"stdin",    stdin_read_r,            NULL},
	{"stdout",            NULL,  stdout_write_r},
	{"stderr",            NULL,  stderr_write_r},

	{"usb_cdc", USB_CDC_read_r, USB_CDC_write_r},

	{0},
};
