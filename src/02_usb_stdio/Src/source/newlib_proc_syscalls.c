/*
 * newlib_proc_syscalls.c
 *
 *  Created on: 3 wrz 2017
 *      Author: Kamil Cukrowski <kamilcukrowski __at__ gmail.com>
 *     License: jointly under MIT License and the Beerware License.
 */
#include <stdlib.h>
#include <reent.h>
#include <errno.h>
#include <limits.h>
#include <cmsis_os.h>
#include <unistd.h>

// board specific
#include "stm32f1xx_hal.h" // NVIC_SystemReset(); __BKPT(value)

/* Empty environment definition */
char *__env[1] = { 0 };
char **environ = __env;


/* ************************ */
/* Process control syscalls */
/* ************************ */

void
_exit(int code) {
	volatile int status; /* volatile to prevent optimizations to remove the variable from memory */
	status = code;
	(void)status; /* Suppress compiler warnings about unused variable */

	if ( xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED ) {
		vTaskDelete(NULL);
	}

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
	ptr->_errno = ENOTSUP;
	return -1;
}

int
kill(int  pid, int  sig) {
  /* Not supported */
  (void)pid; /* Suppress compiler warnings about unused parameters */
  (void)sig;

  if ( pid == getpid() ) {
	  _exit(-1); // end ourselves
  } else {
  }

  // ptr->_errno = EINVAL;
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
