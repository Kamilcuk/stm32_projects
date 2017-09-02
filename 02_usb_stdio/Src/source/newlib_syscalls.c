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
#include <reent.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "newlib_devoptab.h" // deopttab

// board specific
#include "stm32f1xx_hal.h" // NVIC_SystemReset(); __BKPT(value) SCB->ICSR

#define MAX_OPEN_DEVICES 255

/* Empty environment definition */
char *__env[1] = { 0 };
char **environ = __env;

/* Align all sbrk arguments to this many bytes */
#define DYNAMIC_MEMORY_ALIGN 4

/* ************************ */
/* Process control syscalls */
/* ************************ */

void
_exit(int code) {
  volatile int status; /* volatile to prevent optimizations to remove the variable from memory */
  status = code;
  (void)status; /* Suppress compiler warnings about unused variable */

  /* See local variable `status` during debugger break. */
  __BKPT(1);
  NVIC_SystemReset();
  while(1);
}

int
_fork_r(struct _reent *ptr) {
  /* return "not supported" */
  ptr->_errno = ENOTSUP;
  return -1;
}

int
_execve_r(struct _reent *ptr, const char *name, char *const *argv, char *const *env) {
  /* Not supported */
  (void)name; /* Suppress compiler warnings about unused parameters */
  (void)argv;
  (void)env;

  ptr->_errno = ENOMEM;
  return -1;
}

int
_kill_r(struct _reent *ptr, int  pid, int  sig) {
  /* Not supported */
  (void)pid; /* Suppress compiler warnings about unused parameters */
  (void)sig;

  ptr->_errno = EINVAL;
  return -1;
}

pid_t
_getpid(void)
{
  /** \todo Return some process identifier on getpid() */
  return 1;
}

clock_t
_times_r(struct _reent *ptr, struct tms *buf)
{
  /* Not supported, yet */
  (void)buf; /* Suppress compiler warnings about unused parameters */

  ptr->_errno = EACCES;
  return  -1;
}

int
_wait_r(struct _reent *ptr, int *status)
{
  /* Not supported, yet */
  (void)status; /* Suppress compiler warnings about unused parameters */

  ptr->_errno = ECHILD;
  return -1;
}

/* ******************************** */
/* File descriptor related syscalls */
/* ******************************** */

static int devoptab_list_len() {
	static int i = -1;
	if ( i == -1 ) {
		for(i = 0; devoptab_list[i].name != NULL; ++i) {
			continue;
		}
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

/* ********************************** */
/* Memory management related syscalls */
/* ********************************** */

#define assert_not_in_interrupt() assert_param(  (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) == 0 )

extern int _end; // set by linker
extern int _estack; // end of RAM
static const uintptr_t heap_end = (uintptr_t)&_estack;

#define USE_C11
#ifndef USE_C11

#include <cmsis_os.h> // freertos

static uintptr_t current_break = (uintptr_t)&_end;

void *_sbrk_r(struct _reent *ptr, ptrdiff_t increment)
{
	assert_not_in_interrupt(); // never call this from ISR

	/* Align memory increment to nearest DYNAMIC_MEMORY_ALIGN bytes upward */
	const ptrdiff_t remainder = increment % DYNAMIC_MEMORY_ALIGN;
	if ( remainder != 0 ) {
		increment += DYNAMIC_MEMORY_ALIGN - remainder;
	}

	taskENTER_CRITICAL();

	current_break += increment;
	if ( current_break > heap_end ) {
		/* out of memory */
		current_break -= increment;
		taskEXIT_CRITICAL();
		ptr->_errno = ENOMEM;
		return (void*)-1;
	}
	void * const ret = (void*)current_break;

	taskEXIT_CRITICAL();

	return ret;
}

#else

// C11 atomic blocking implementation

#include <stdatomic.h>
static atomic_uintptr_t current_break = ATOMIC_VAR_INIT( (uintptr_t)&_end );

void *_sbrk_r(struct _reent *ptr, ptrdiff_t increment)
{
	assert_not_in_interrupt(); // never call this from ISR

	/* Align memory increment to nearest DYNAMIC_MEMORY_ALIGN bytes upward */
	const ptrdiff_t remainder = increment % DYNAMIC_MEMORY_ALIGN;
	if ( remainder != 0 ) {
		increment += DYNAMIC_MEMORY_ALIGN - remainder;
	}

	uintptr_t tmp = atomic_fetch_add_explicit(&current_break, increment, memory_order_acquire);
	if ( tmp > heap_end ) {
		/* out of memory */
		atomic_fetch_sub_explicit(&current_break, increment, memory_order_release);
		ptr->_errno = ENOMEM;
		return (void*)-1;
	}
	atomic_exchange_explicit(&current_break, tmp, memory_order_release);
	return (void*)tmp;
}
#endif
