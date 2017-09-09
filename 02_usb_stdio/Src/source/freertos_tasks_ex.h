/*
 * freertos_tasks_ex.h
 *
 *  Created on: 3 wrz 2017
 *      Author: Kamil Cukrowski <kamilcukrowski __at__ gmail.com>
 *     License: jointly under MIT License and the Beerware License.
 */

#ifndef SRC_SOURCE_FREERTOS_TASKS_EX_H_
#define SRC_SOURCE_FREERTOS_TASKS_EX_H_

#include "cmsis_os.h"

int xTaskFork(
		const char * const pcName,
		TaskHandle_t * const pxCreatedTask );

void xTaskExecve(
		TaskHandle_t * const pxTask,
		TaskFunction_t pxTaskCode,
		const uint32_t ulStackDepth,
		void * const pvParameters );

BaseType_t xTaskForkExecve(TaskFunction_t func, char *const *argv);

#endif /* SRC_SOURCE_FREERTOS_TASKS_EX_H_ */
