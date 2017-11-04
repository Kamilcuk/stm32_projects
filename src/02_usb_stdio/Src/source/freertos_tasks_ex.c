/*
 * port_ex.c
 *
 *  Created on: 3 wrz 2017
 *      Author: Kamil Cukrowski <kamilcukrowski __at__ gmail.com>
 *     License: jointly under MIT License and the Beerware License.
 */

#include "../../STM32Cube_FW_F1/Middlewares/Third_Party/FreeRTOS/Source/tasks.c"

StackType_t *pxPortAddStack(
		StackType_t *pxTopOfStack,
		TaskFunction_t pxCode, void *pvParameters,
		TaskFunction_t pxReturn )
{
	/* Simulate the stack frame as it would be created by a context switch
	interrupt. */
	pxTopOfStack--; /* Offset added to account for the way the MCU uses the stack on entry/exit of interrupts. */
	*pxTopOfStack = 0x01000000UL;	/* xPSR */
	pxTopOfStack--;
	*pxTopOfStack = ( ( StackType_t ) pxCode ) & 0xfffffffeUL;	/* PC */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) pxReturn;	/* LR */
	pxTopOfStack -= 5;	/* R12, R3, R2 and R1. */
	*pxTopOfStack = ( StackType_t ) pvParameters;	/* R0 */
	pxTopOfStack -= 8;	/* R11, R10, R9, R8, R7, R6, R5 and R4. */

	return pxTopOfStack;
}

void prvFork(TCB_t * const pxNewTCB)
{
	register volatile void *pxReturn asm ("lr");
	if ( pxNewTCB != NULL ) {
		pxNewTCB->pxTopOfStack = pxPortAddStack( (StackType_t *) pxNewTCB->pxTopOfStack,
				(TaskFunction_t)prvFork, NULL, pxReturn );
	}
}

static void prvCopyTask(
		const char * const pcName,
		TCB_t * const pxFromTask,
		TaskHandle_t * const pxToTask,
		const uint32_t usStackDepth,
		TCB_t * const pxNewTCB)
{
	strcpy(pxNewTCB->pcTaskName, pcName);
	memcpy( pxNewTCB->pxStack , pxFromTask->pxStack, usStackDepth );
	pxNewTCB->pxTopOfStack = pxNewTCB->pxStack + ( pxFromTask->pxTopOfStack - pxFromTask->pxStack );
	prvFork(pxNewTCB);
	*pxToTask = (TaskHandle_t)pxNewTCB;
}


#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )

static BaseType_t xTaskCreateFork(
		const char * const pcName,
		TCB_t * const pxFromTask,
		TaskHandle_t * const pxToTask )
{
	const uint32_t usStackDepth = configMINIMAL_STACK_SIZE;
	TCB_t *pxNewTCB;
	BaseType_t xReturn;

	/* If the stack grows down then allocate the stack then the TCB so the stack
		does not grow into the TCB.  Likewise if the stack grows up then allocate
		the TCB then the stack. */
#if( portSTACK_GROWTH > 0 )
	{
		/* Allocate space for the TCB.  Where the memory comes from depends on
			the implementation of the port malloc function and whether or not static
			allocation is being used. */
		pxNewTCB = ( TCB_t * ) pvPortMalloc( sizeof( TCB_t ) );

		if( pxNewTCB != NULL )
		{
			/* Allocate space for the stack used by the task being created.
				The base of the stack memory stored in the TCB so the task can
				be deleted later if required. */
			pxNewTCB->pxStack = ( StackType_t * ) pvPortMalloc( ( ( ( size_t ) usStackDepth ) * sizeof( StackType_t ) ) ); /*lint !e961 MISRA exception as the casts are only redundant for some ports. */

			if( pxNewTCB->pxStack == NULL )
			{
				/* Could not allocate the stack.  Delete the allocated TCB. */
				vPortFree( pxNewTCB );
				pxNewTCB = NULL;
			}
		}
	}
#else /* portSTACK_GROWTH */
	{
		StackType_t *pxStack;

		/* Allocate space for the stack used by the task being created. */
		pxStack = ( StackType_t * ) pvPortMalloc( ( ( ( size_t ) usStackDepth ) * sizeof( StackType_t ) ) ); /*lint !e961 MISRA exception as the casts are only redundant for some ports. */

		if( pxStack != NULL )
		{
			/* Allocate space for the TCB. */
			pxNewTCB = ( TCB_t * ) pvPortMalloc( sizeof( TCB_t ) ); /*lint !e961 MISRA exception as the casts are only redundant for some paths. */

			if( pxNewTCB != NULL )
			{
				/* Store the stack location in the TCB. */
				pxNewTCB->pxStack = pxStack;
			}
			else
			{
				/* The stack cannot be used as the TCB was not created.  Free
					it again. */
				vPortFree( pxStack );
			}
		}
		else
		{
			pxNewTCB = NULL;
		}
	}
#endif /* portSTACK_GROWTH */

	if( pxNewTCB != NULL )
	{
#if( tskSTATIC_AND_DYNAMIC_ALLOCATION_POSSIBLE != 0 )
		{
			/* Tasks can be created statically or dynamically, so note this
				task was created dynamically in case it is later deleted. */
			pxNewTCB->ucStaticallyAllocated = tskDYNAMICALLY_ALLOCATED_STACK_AND_TCB;
		}
#endif /* configSUPPORT_STATIC_ALLOCATION */

		//prvInitialiseNewTask( pxTaskCode, pcName, ( uint32_t ) usStackDepth, pvParameters, uxPriority, pxCreatedTask, pxNewTCB, NULL );
		prvCopyTask( pcName, pxFromTask, pxToTask, usStackDepth, pxNewTCB );

		prvAddNewTaskToReadyList( pxNewTCB );
		xReturn = pdPASS;
	}
	else
	{
		xReturn = errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
	}

	return xReturn;
}

int xTaskFork(
		const char * const pcName,
		TaskHandle_t * const pxCreatedTask )
{
	TaskHandle_t *pxToTask = NULL;
	if ( xTaskCreateFork(pcName, pxCurrentTCB, pxToTask) != pdPASS ) {
		return -1;
	}
	taskYIELD();
	if ( pxCreatedTask != NULL ) {
		*pxCreatedTask = pxToTask;
	}
	if ( ( void * ) pxToTask != pxCurrentTCB ) {
		return 1; // parent
	}
	return 0; // child
}

#endif /* configSUPPORT_DYNAMIC_ALLOCATION */

void xTaskExecve(
		TaskHandle_t * const pxTask,
		TaskFunction_t pxTaskCode,
		const uint32_t ulStackDepth,
		void * const pvParameters )
{
	TCB_t *pxTCB;
	pxTCB = prvGetTCBFromHandle( pxTask );
	prvInitialiseNewTask( pxTaskCode,  pcTaskGetName(pxTask), ulStackDepth,
			pvParameters, pxTCB->uxPriority, NULL, pxTCB, NULL);
}

BaseType_t xTaskForkExecve(TaskFunction_t func, char *const *argv)
{
	if ( xTaskCreate(func, "1", configMINIMAL_STACK_SIZE,
			(void * const)argv, 0, NULL) != pdPASS ) {
		return errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
	}
	return pdPASS;
}
