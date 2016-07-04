/*
 *	Amazon IOT - Smart Home External Hardware Driver
 *
 *	This is the implementation file for the externel hardware driver
 *
 *	-	LED Stripe
 *	-	Light sensor
 *	-	Heating Element
 *
 *	Simon Schwan, Alexander Jähnel, Christian Gerson
 *
 *	Version:	1.0
 *				06.2016
 *
 *	To use the LED driver:
 *	- to init the driver call:	ledInit();
 *  - to set a color call:		ledSetColor(v1,0,0);
 *  - to show the color call:	ledShow();
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

#include "driver.h"

/*
 * 		led stripe part
 */
 
void ledInit(){
    //
    // Enable the SPI module clock
    //
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI,PRCM_RUN_MODE_CLK);

    //
    // Reset the peripheral
    //
    MAP_PRCMPeripheralReset(PRCM_GSPI);

    //
    // Reset SPI
    //
    MAP_SPIReset(GSPI_BASE);

    //
    // Configure SPI interface
    //
    MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                     SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVEHIGH |
                     SPI_WL_8));

    //
    // Enable SPI for communication
    //
    MAP_SPIEnable(GSPI_BASE);
}

void ledSetColor(uint8_t r, uint8_t g, uint8_t b){
	static uint8_t g_ucTxBuff[3];
	static uint8_t g_ucRxBuff[3];
    //
    // Send the string to slave. Chip Select(CS) needs to be
    // asserted at start of transfer and deasserted at the end.
    //
    g_ucTxBuff[0] = r;	//R
    g_ucTxBuff[1] = g;	//G
    g_ucTxBuff[2] = b;	//B
    MAP_SPITransfer(GSPI_BASE,g_ucTxBuff,g_ucRxBuff,3,
            SPI_CS_ENABLE|SPI_CS_DISABLE);
}

void ledShow(){
    //we need a pause of 500us between each cycle
    UtilsDelay(10000);
}

/*
 * 		heating element part
 */

void heatON(){
	//switch the heating element on
	MAP_GPIOPinWrite(GPIOA3_BASE, GPIO_PIN_4, GPIO_PIN_4);
}

void heatOff(){
	//switch the heating elment off
    MAP_GPIOPinWrite(GPIOA3_BASE, GPIO_PIN_4, 0);
}

/*
 * 		light sensor part
 */

void lightSensorInit(){
	
	MAP_PinTypeADC(PIN_58,PIN_MODE_255);
    //
    // Configure ADC timer which is used to timestamp the ADC data samples
    //
    MAP_ADCTimerConfig(ADC_BASE,2^17);

    //
    // Enable ADC timer which is used to timestamp the ADC data samples
    //
    MAP_ADCTimerEnable(ADC_BASE);

    //
    // Enable ADC module
    //
    MAP_ADCEnable(ADC_BASE);
}

uint16_t lightSensorGetValue(uint32_t samples){

	uint32_t	 	haveSample=0;
	float 			sampleValue=0;
	unsigned long	ulSample;

	//
    // Enable ADC channel
    //
    MAP_ADCChannelEnable(ADC_BASE, ADC_CH_1);

    //discard first 10 samples
	while(haveSample < 10){
    	if(MAP_ADCFIFOLvlGet(ADC_BASE, ADC_CH_1)){
    		ulSample = MAP_ADCFIFORead(ADC_BASE, ADC_CH_1);
    		haveSample++;
    	}
    }

	haveSample = 0;
	while(haveSample < samples){
    	if(MAP_ADCFIFOLvlGet(ADC_BASE, ADC_CH_1)){
    		ulSample = MAP_ADCFIFORead(ADC_BASE, ADC_CH_1);
    		sampleValue += ((ulSample >> 2) & 0x0FFF);
    		haveSample++;
    	}
    }

    MAP_ADCChannelDisable(ADC_BASE, ADC_CH_1);

    //MAP_ADCDisable(ADC_BASE);

    //MAP_ADCTimerDisable(ADC_BASE);
	
    //divide samples
    sampleValue /= samples;

	return sampleValue;
}

/*
 * 		external switch part
 */

 void extSwitchWaitForInput(){
	uint8_t buttonPressed = 0;
	while(!buttonPressed){
		if(!(MAP_GPIOPinRead(GPIOA0_BASE, GPIO_PIN_7) & GPIO_PIN_7)){
			buttonPressed = 1;
		}
	}
 }
 
 uint8_t extSwitchGetValue(){
	uint8_t retVal = !(MAP_GPIOPinRead(GPIOA0_BASE, GPIO_PIN_7) & GPIO_PIN_7);
	return retVal;
 }
