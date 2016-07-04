/*
 *	Amazon IOT - Smart Home External Hardware Driver
 *
 *	This is the header file for the externel hardware driver
 *
 *	-	LED Stripe
 *	-	Light sensor
 *	-	Heating Element
 *	-	external switch
 *
 *	Simon Schwan, Alexander Jähnel, Christian Gerson
 *
 *	Version:	1.0
 *				06.2016
 */
 
 /*
  *		Please add the following lines to pinmux.c
  *
		//
		// Enable Peripheral Clocks 
		//
		MAP_PRCMPeripheralClkEnable(PRCM_GSPI, PRCM_RUN_MODE_CLK);
		MAP_PRCMPeripheralClkEnable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK);
		MAP_PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);
		
		//
		// Configure PIN_05 for SPI0 GSPI_CLK
		//
		MAP_PinTypeSPI(PIN_05, PIN_MODE_7);

		//
		// Configure PIN_07 for SPI0 GSPI_MOSI
		//
		MAP_PinTypeSPI(PIN_07, PIN_MODE_7);

		//
		// Configure PIN_18 for GPIOOutput
		//
		MAP_PinTypeGPIO(PIN_18, PIN_MODE_0, false);
		MAP_GPIODirModeSet(GPIOA3_BASE, GPIO_PIN_4, GPIO_DIR_MODE_OUT);
		
		//
		// Configure PIN_62 for GPIOInput
		//
		MAP_PinTypeGPIO(PIN_62, PIN_MODE_0, false);
		MAP_GPIODirModeSet(GPIOA0_BASE, GPIO_PIN_7, GPIO_DIR_MODE_IN);
		
  *		
  */
  
#include <stdint.h>
  
// Driverlib includes
#include "utils.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_adc.h"
#include "rom_map.h"
#include "prcm.h"
#include "pin.h"
#include "adc.h"
#include "spi.h"
#include "gpio.h"


#define SPI_IF_BIT_RATE  100000

/*
 * 		led stripe part
 */
void ledInit();
void ledSetColor(uint8_t r, uint8_t g, uint8_t b);
void ledShow();

/*
 * 		heating element part
 */
void heatON();
void heatOff();

/*
 * 		light sensor part
 */
void lightSensorInit();
uint16_t lightSensorGetValue(uint32_t samples);

/*
 *		external switch
 */
/*
 *	wait until the user pressed the light button
 */
 void extSwitchWaitForInput();
 /*
  *	0 = button not pressed
  *	1 = button pressed
  */
 uint8_t extSwitchGetValue();
