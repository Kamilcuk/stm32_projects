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
#include "usbd_def.h"
#include "usbd_cdc_if.h"
#include <errno.h>
#include <stdatomic.h>

#ifdef MIN
#undef MIN
#endif
#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })


#ifdef MAX
#undef MAX
#endif
#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define USB_CDC_Mutual_exclusion 1

extern USBD_HandleTypeDef hUsbDeviceFS;

static volatile uint8_t CDC_Receive_Buff[2*USB_CDC_RX_DATA_SIZE];
static const uint32_t CDC_Receive_size = sizeof(CDC_Receive_Buff)/sizeof(*CDC_Receive_Buff);
static volatile uint32_t CDC_Receive_len;

#if USB_CDC_Mutual_exclusion

static atomic_flag USB_CDC_read_mutex = ATOMIC_FLAG_INIT;
static atomic_flag USB_CDC_write_mutex = ATOMIC_FLAG_INIT;

#define USB_CDC_Mutual_exclusion_enter(x) while( atomic_flag_test_and_set( & x ) );
#define USB_CDC_Mutual_exclusion_exit(x)  atomic_flag_clear( & x );

#else
#define USB_CDC_Mutual_exclusion_enter(x)
#define USB_CDC_Mutual_exclusion_exit(x)
#endif

bool USB_CDC_Configured(void)
{
	return hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED;
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

	USB_CDC_Mutual_exclusion_enter( USB_CDC_read_mutex );

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

	USB_CDC_Mutual_exclusion_exit( USB_CDC_read_mutex );

	return len;
}

ssize_t USB_CDC_write_r(struct _reent *r, int fd, const void *buf, size_t cnt)
{
	uint8_t ret;
	if ( !USB_CDC_Configured() ) {
		r->_errno = EAGAIN;
		return -1;
	}

	USB_CDC_Mutual_exclusion_enter( USB_CDC_write_mutex );

	const size_t len = MIN(cnt, USB_CDC_TX_DATA_SIZE);
	while( (ret = CDC_Transmit_FS((uint8_t*)buf, len)) == USBD_BUSY );
	if ( ret != USBD_OK ) {
		r->_errno = EIO;
		return -1;
	}

	USB_CDC_Mutual_exclusion_exit( USB_CDC_write_mutex );

	return len;
}


__weak void CDC_Receive_FS_callback(uint8_t *Buf, uint32_t Buflen) // referenced by CDC_Receive_FS
{
	const uint32_t len = MIN( CDC_Receive_size - CDC_Receive_len , Buflen);
	memcpy((uint8_t*)&CDC_Receive_Buff[CDC_Receive_len], Buf, len);
	CDC_Receive_len += len;
}
