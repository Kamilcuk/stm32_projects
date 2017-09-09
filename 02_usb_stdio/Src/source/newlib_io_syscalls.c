/*
 * newlib_syscalls.c
 *
 *  Created on: 28.08.2017
 *      Author: Kamil Cukrowski
 *
 *
 * based on http://www.eistec.se/docs/contiki/a01137_source.html
 * and based on multiple examples on the net
 */
#include <sys/reent.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/cdefs.h>
#include <stdlib.h>
#include <reent.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "newlib_devoptab.h" // deopttab

#define MAX_OPEN_DEVICES 255


/* ******************************** */
/* File descriptor related syscalls */
/* ******************************** */

static int devoptab_list_len() {
	int i;
	for(i = 0; devoptab_list[i].name != NULL; ++i) {
		continue;
	}
	return i;
}

int
_open_r(struct _reent *ptr, const char *name, int flags, int mode) {
	unsigned int i;
	for(i = 0; devoptab_list[i].name != NULL; ++i) {
		if ( strcmp(devoptab_list[i].name, name) == 0 ) {
			if ( devoptab_list[i].open_r ) {
				if ( devoptab_list[i].open_r(ptr, name, flags, mode) != 0 ) {
					ptr->_errno = EIO;
					return -1;
				}
			}
			return i;
		}
	}
	ptr->_errno = ENODEV;
	return -1;
}

#define NEWLIB_DEVOPTAB_LIST_POINTER( TYPE , CALLBACK , ARGS_DEF , ARGS_CALL ) \
TYPE \
_ ## CALLBACK ARGS_DEF \
{ \
	if ( fd < 0 || fd > devoptab_list_len() ) { \
		/* invalid file descriptor */ \
		ptr->_errno = EBADF; \
		return -1; \
	} \
	if ( devoptab_list[fd].CALLBACK == NULL ) { \
		/* Function not implemented */ \
		ptr->_errno = ENOSYS; \
		return -1; \
	} \
	/* Call method from device operations table */ \
	return devoptab_list[fd].CALLBACK ARGS_CALL ; \
}

NEWLIB_DEVOPTAB_LIST_POINTER(
		int,
		close_r,
		(struct _reent *ptr, int fd),
		(ptr, fd)
)

NEWLIB_DEVOPTAB_LIST_POINTER(
		_ssize_t,
		read_r,
		(struct _reent *ptr, int fd, void *buf, size_t cnt),
		(ptr, fd, buf, cnt)
)

NEWLIB_DEVOPTAB_LIST_POINTER(
		_ssize_t,
		write_r,
		(struct _reent *ptr, int fd, const void *buf, size_t cnt),
		(ptr, fd, buf, cnt)
)

NEWLIB_DEVOPTAB_LIST_POINTER(
		_off_t,
		lseek_r,
		(struct _reent *ptr, int fd, off_t offset, int whence),
		(ptr, fd, offset, whence)
)

NEWLIB_DEVOPTAB_LIST_POINTER(
		_ssize_t,
		fcntl_r,
		(struct _reent *ptr, int fd, int cmd, int arg),
		(ptr, fd, cmd, arg)
)


/* **************************** */
/* File system related syscalls */
/* **************************** */

int
_fstat_r(struct _reent *ptr, int fd, struct stat *st) {
	/* not supported, yet */
	(void)fd; /* Suppress compiler warnings about unused parameters */
	(void)st;
	ptr->_errno = ENOENT;
	return -1;
}

int
_stat_r(struct _reent *ptr, const char *file, struct stat *st) {
	/* not supported, yet */
	(void)file; /* Suppress compiler warnings about unused parameters */
	(void)st;
	ptr->_errno = ENOENT;
	return -1;
}

int
_link_r(struct _reent *ptr, const char *old, const char *new)
{
	/* not supported, yet */
	(void)old; /* Suppress compiler warnings about unused parameters */
	(void)new;
	ptr->_errno = EMLINK;
	return -1;
}

int
_unlink_r(struct _reent *ptr, const char *name)
{
	/* not supported, yet */
	(void)name; /* Suppress compiler warnings about unused parameters */
	ptr->_errno = ENOENT;
	return -1;
}
