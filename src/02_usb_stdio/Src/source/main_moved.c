/*
 * main_moved.c
 *
 *  Created on: 26.08.2017
 *      Author: Kamil Cukrowski
 */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/unistd.h>
#define __need_getopt_newlib
#include <getopt.h>
#include <string.h>
#include "main.h"
#include "stm32f1xx_hal.h"
#include "usbd_cdc_if_ex.h"
#include "cmsis_os.h"
#include "freertos_tasks_ex.h"

FILE *usb;

void StartDefaultTask(const void *arg)
{
	(void)arg;
	int i=0;
	for(;;) {
		fprintf(usb, "i=%d czesc z %lx / %s \n", i++, arg, pcTaskGetName(NULL));
		HAL_Delay(1000);
	}
}

void mainThread(const void *arg)
{
	(void)arg;

	HAL_Delay(100);
	HAL_Delay(100);
	HAL_Delay(100);

	for(;;) {
		HAL_Delay(1000);
		HAL_Delay(arg);
	}
}

void mallocTest(const void *arg)
{
	uint32_t arg2 = (uint32_t)arg;
	while( usb == NULL );
	for(;;) {
		void * la = malloc(arg2);
		fprintf(usb, "a%lx %ld\n", la, arg2);
		free( la );
		fprintf(usb, "f%lx %ld\n", la, arg2);
	}
}

struct myThread_s {
	os_pthread pthread;
	void *arg;
	osPriority tpriority;
};
x
void myThreadStart(struct myThread_s *myThreads, int len)
{
	static char buff[] = "@@";
	for(; len ; --len, ++myThreads ) {
		buff[1]++;
		osThreadDef_t osThreadDef = {
				buff, myThreads->pthread, myThreads->tpriority,
					0, configMINIMAL_STACK_SIZE
		};
		fprintf(usb, "osThreadCreate({%s,%lx,%lx,%lx,%lx},%lx);\n",
				osThreadDef.name, osThreadDef.pthread, osThreadDef.tpriority,
				osThreadDef.instances, osThreadDef.stacksize, myThreads->arg);
		if ( osThreadCreate(&osThreadDef, myThreads->arg) == NULL ) {
			assert_param(false);
		}
	}
}

int main()
{
	volatile static uint32_t incredible_stupid = 50;

	MX_Init();

	setvbuf(stdin, NULL, _IOLBF, 16);
	setvbuf(stdout, NULL, _IOLBF, 16);
	setvbuf(stderr, NULL, _IONBF, 3);

	usb = fopen("usb_cdc", "r+");
	setvbuf(usb, NULL, _IOLBF, 100);
	HAL_Delay(1000);
	fprintf(usb, "Czesc do usb!\n");
	HAL_Delay(1005);

	{
		struct myThread_s myThreads[] = {
				{ StartDefaultTask,  (void*)0, osPriorityNormal, },
				{ StartDefaultTask,  (void*)10, osPriorityNormal, },
				{ StartDefaultTask,  (void*)20, osPriorityNormal, },
				{ mainThread,  (void*)100, osPriorityNormal, },
				{ mainThread,  (void*)200, osPriorityNormal, },
				{ mainThread,  (void*)300, osPriorityNormal, },
				{ mainThread,  (void*)400, osPriorityNormal, },
				{ mainThread,  (void*)500, osPriorityNormal, },
				{ mallocTest,       (void*)10, osPriorityNormal, },
				{ mallocTest,       (void*)20, osPriorityNormal, },
				{ mallocTest,       (void*)30, osPriorityNormal, },
		};
		myThreadStart( myThreads , sizeof(myThreads)/sizeof(*myThreads));
	}

	osKernelStart();

	assert_param(false);

	return 1;
}
