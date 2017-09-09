/*
 * usbd_cdc_if_rx.c
 *
 *  Created on: 26.08.2017
 *      Author: Kamil Cukrowski
 */
#include <sys/cdefs.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdatomic.h>
#include "min_max.h"

#include "usbd_cdc_if.h"
#include "usb_device.h" // hUsbDeviceFS

/* config --------------------------------------------------------------------- */

#define USB_CDC_Mutual_exclusion 1

/* private variables ----------------------------------------------------------- */

static volatile uint8_t CDC_Receive_Buff[2*USB_CDC_RX_DATA_SIZE];
static const uint32_t CDC_Receive_size = sizeof(CDC_Receive_Buff)/sizeof(*CDC_Receive_Buff);
static volatile uint32_t CDC_Receive_len;

#if USB_CDC_Mutual_exclusion

static atomic_flag USB_CDC_mutex = ATOMIC_FLAG_INIT;

#define USB_CDC_Mutual_exclusion_enter() while( atomic_flag_test_and_set( & USB_CDC_mutex ) );
#define USB_CDC_Mutual_exclusion_exit()  atomic_flag_clear( & USB_CDC_mutex );

#else
#define USB_CDC_Mutual_exclusion_enter(x)
#define USB_CDC_Mutual_exclusion_exit(x)
#endif

/* exported functions ----------------------------------------------------------- */

bool USB_CDC_Configured(void)
{
	return hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED;
}

int USB_CDC_open_r(struct _reent *ptr, const char *path, int flags, int mode)
{
	USB_CDC_Mutual_exclusion_enter();

	MX_USB_DEVICE_Init();

	while( USB_CDC_Configured() == false );

	USB_CDC_Mutual_exclusion_exit();
	return 0;
}

int USB_CDC_close_r(struct _reent *ptr, int fd)
{
	USB_CDC_Mutual_exclusion_enter();

	if ( USBD_Stop(&hUsbDeviceFS) != USBD_OK ) {
		ptr->_errno = EIO;
		return -1;
	}

	USB_CDC_Mutual_exclusion_exit();
	return 0;
}

ssize_t USB_CDC_read_r(struct _reent *r, int fd, void *buf, size_t cnt)
{
	UNUSED(fd);
	if ( !USB_CDC_Configured() ) {
		r->_errno = EAGAIN;
		return -1;
	}

	PCD_HandleTypeDef *pData = (PCD_HandleTypeDef*)hUsbDeviceFS.pData;
	if ( !pData ) {
		r->_errno = EIO;
		return -1;
	}

	USB_CDC_Mutual_exclusion_enter();

#if 0
	while( CDC_Receive_len == 0 ); //blocking wait for something to read
#endif

	const uint32_t CDC_Receive_len_sav = CDC_Receive_len; // volatile operation, no need to disable interrupts
	if ( CDC_Receive_len_sav == 0 ) {
		return 0;
	}
	const size_t len = MIN( CDC_Receive_len_sav , cnt );
	// we *know* that CDC_Receive_FS_callback touches only CDC_Receive_Buff[len+1, ...]
	memcpy(buf, (uint8_t*)CDC_Receive_Buff, len);

	__HAL_PCD_DISABLE(pData);
	memmove((uint8_t*)&CDC_Receive_Buff[0], (uint8_t*)&CDC_Receive_Buff[len], CDC_Receive_len - len);
	CDC_Receive_len -= len;
	__HAL_PCD_ENABLE(pData);

	USB_CDC_Mutual_exclusion_exit();
	return len;
}

ssize_t USB_CDC_write_r(struct _reent *ptr, int fd, const void *buf, size_t cnt)
{
	if ( !USB_CDC_Configured() ) {
		if ( ptr ) ptr->_errno = EAGAIN;
		return -1;
	}

	size_t len;

	USB_CDC_Mutual_exclusion_enter();
	{
		len = MIN(cnt, USB_CDC_TX_DATA_SIZE);
		uint8_t ret;
		while( (ret = CDC_Transmit_FS((uint8_t*)buf, len)) == USBD_BUSY );
		if ( ret != USBD_OK ) {
			if ( ptr ) ptr->_errno = EIO;
			len = -1;
		}
	}
	USB_CDC_Mutual_exclusion_exit();

	return len;
}


__weak void CDC_Receive_FS_callback(uint8_t *Buf, uint32_t Buflen) // referenced by CDC_Receive_FS
{
	const uint32_t len = MIN( CDC_Receive_size - CDC_Receive_len , Buflen);
	memcpy((uint8_t*)&CDC_Receive_Buff[CDC_Receive_len], Buf, len);
	CDC_Receive_len += len;
}
