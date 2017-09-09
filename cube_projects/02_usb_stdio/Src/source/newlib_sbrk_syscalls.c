/*
 * newlib_sbrk_syscalls.c
 *
 *  Created on: 3 wrz 2017
 *      Author: Kamil Cukrowski <kamilcukrowski __at__ gmail.com>
 *     License: jointly under MIT License and the Beerware License.
 */
/* ********************************** */
/* Memory management related syscalls */
/* ********************************** */

#include <reent.h>
#include <errno.h>

#include "stm32f1xx_hal.h" //  SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk

/* Align all sbrk arguments to this many bytes */
#define DYNAMIC_MEMORY_ALIGN 4

#define assert_not_in_interrupt() assert_param(  (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) == 0 )

extern const int _end; // set by linker
extern const int _estack; // end of RAM
static const uintptr_t heap_start = (const uintptr_t)&_end;
static const uintptr_t heap_end = (const uintptr_t)&_estack;

#if 1
#define USE_C11
#ifndef USE_C11

#include <cmsis_os.h> // freertos

static uintptr_t current_break = (uintptr_t)&_end;

void *_sbrk_r(struct _reent *ptr, ptrdiff_t increment)
{
	assert_not_in_interrupt(); // never call this from ISR

	taskENTER_CRITICAL();
	{
		void *ret = (void*)current_break;
		current_break += increment;
		if ( current_break > heap_end ) {
			/* out of memory */
			ptr->_errno = ENOMEM;
			ret = (void*)-1;
		}
	}
	taskEXIT_CRITICAL();

	return ret;
}

#else

// C11 atomic blocking implementation

#include <stdatomic.h>
static atomic_uintptr_t current_break = ATOMIC_VAR_INIT( (uintptr_t)&_end );

uintptr_t getCurrentBreak(void)
{
	return atomic_load_explicit(&current_break, memory_order_relaxed);
}

void *_sbrk_r(struct _reent *ptr, ptrdiff_t increment)
{
	assert_not_in_interrupt(); // never call this from ISR

	uintptr_t ret = atomic_fetch_add_explicit(&current_break, increment, memory_order_acquire);
	if ( ret > heap_end ) {
		/* out of memory */
		atomic_fetch_sub_explicit(&current_break, ret, memory_order_release);
		ptr->_errno = ENOMEM;
		ret = -1;
	}
	atomic_fetch_add_explicit(&current_break, 0, memory_order_release);

	return (void *)ret;
}
#endif
#endif
