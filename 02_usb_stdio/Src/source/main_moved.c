/*
 * main_moved.c
 *
 *  Created on: 26.08.2017
 *      Author: Kamil Cukrowski
 */
#include "main.h"
#include <stdio.h>
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "usbd_cdc_if_ex.h"
#include <errno.h>


#pragma GCC optimize ("O0")
int main()
{
	int i=0;

	MX_Init();

	while( USB_CDC_Configured() == false );

	setvbuf(stdin, NULL, _IOLBF, 16);
	setvbuf(stdout, NULL, _IOLBF, 16);
	setvbuf(stderr, NULL, _IONBF, 3);

	osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
	defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

	FILE *usb = fopen("usb_cdc", "r+");
	setvbuf(usb, NULL, _IOLBF, 16);

	for(;;) {
		fprintf(usb, "czesc z usb i=%d \n", i++);
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		HAL_Delay(1000);
	}
}
