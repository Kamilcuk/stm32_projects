/*
 * usbd_cdc_if_ex.h
 *
 *  Created on: 26.08.2017
 *      Author: Kamil Cukrowski
 */

#ifndef USBD_CDC_IF_EX_H_
#define USBD_CDC_IF_EX_H_

#include <sys/reent.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>

bool USB_CDC_Configured(void);

int USB_CDC_open_r(struct _reent *ptr, const char *path, int flags, int mode);
int USB_CDC_close_r(struct _reent *ptr, int fd);
ssize_t USB_CDC_read_r(struct _reent *r, int fd, void *buf, size_t cnt);
ssize_t USB_CDC_write_r(struct _reent *r, int fd, const void *buf, size_t cnt);

void CDC_Receive_FS_callback(uint8_t *Buf, uint32_t Buflen);

#endif /* USBD_CDC_IF_EX_H_ */
