/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/


#include <stdio.h>
#include "xil_printf.h"
#include "xiic.h" //for temprature

#include "PmodACL.h"  //for ACL
#include "xil_cache.h" //for ACL

#include "xparameters.h"  //Connecting the hardware to software
#include "sleep.h"   //to use delay functions
#include "math.h" //to use the sqrt function


#include "PmodBLE.h" //for BLE

#include "HealthMonitor.h"


//For Temperature
#define IIC_DEVICE_ID XPAR_AXI_IIC_TEMPRATURE_BASEADDR //for readability and small name for easier coding
#define Temp_address 0x4B //Temp address is 0x4B from datasheets (built in). it has 4 addresses

//For BLE
#include "xuartlite.h"
typedef XUartLite SysUart;
#define SysUart_Send            XUartLite_Send
#define SysUart_Recv            XUartLite_Recv
#define SYS_UART_DEVICE_ID      XPAR_AXI_UARTLITE_0_DEVICE_ID
#define BLE_UART_AXI_CLOCK_FREQ XPAR_CPU_M_AXI_DP_FREQ_HZ


PmodACL acl; //Making our instance to control our ACL pmod
PmodBLE myDevice; //Instance for BLE
SysUart myUart; //Instance for UART (serial terminal)


float Force;
float temperature;


int main()
{
	 EnableCaches();
	 ACL_initialize();
	 SysUartInit();
	 BLE_Init();

	 float x, y, z;



    while(1){

    ACL_ReadAccelG(&acl, &x, &y, &z);
    GetForce(x,y,z);
    //printf("X=%f\tY=%f\tZ=%f\n\r", x, y, z);
    printf("Force: %.4f \n", Force);
    usleep(100000);

    if(Force>4){
    	printf("There is a fall detected\n");
    	sleep(10);
    }

    GetTempValue();
    printf("Temperature: %.4f C°\n", temperature);
    sendFloatBLE(temperature);
    sleep(5);

    }

    DisableCaches();
    return 0;
}






void sendFloatBLE(float value){

	char temp_str[10];
	sprintf(temp_str, "%.4f", value);
	u8 buf[1];

	// Manually calculate the length of the string
	    int len = 0;
	    while (temp_str[len] != '\0') {
	        len++;
	    }

	    for(int i=0;i<=len;i++){
	    	buf[0]=temp_str[i];
	    	BLE_SendData(&myDevice, buf, 1);
	    }

}














//This function is used with  ACL_ReadAccelG(&acl, &x, &y, &z); to get the Force (Magnitude of the acceleration)
// It make use of sqrt function from math.h library. The Force is used to detect if a fall happened because
//The magnitude of acceleration is defined to be the force something subjected to (unit G force)
void GetForce(float x,float y,float z){
	 Force=sqrt((x*x)+(y*y)+(z*z));
}


//Function initialize the ACL PMOD (taken from the ready example)
void ACL_initialize() {

   xil_printf("ACL Initializing");
   ACL_begin(&acl, XPAR_PMODACL_0_AXI_LITE_GPIO_BASEADDR,
         XPAR_PMODACL_0_AXI_LITE_SPI_BASEADDR);
   ACL_SetMeasure(&acl, 0);
   ACL_SetGRange(&acl, ACL_PAR_GRANGE_PM4G);
   ACL_SetMeasure(&acl, 1);
   ACL_CalibrateOneAxisGravitational(&acl, ACL_PAR_AXIS_ZP);
   sleep(1); // After calibration, some delay is required for the new settings
             // to take effect.
}


void SysUartInit()
{
    XUartLite_Initialize(&myUart, SYS_UART_DEVICE_ID);
}

void BLE_Init(){
	 BLE_Begin (
	        &myDevice,
	        XPAR_PMODBLE_0_S_AXI_GPIO_BASEADDR,
	        XPAR_PMODBLE_0_S_AXI_UART_BASEADDR,
	        BLE_UART_AXI_CLOCK_FREQ,
	        115200
	    );
}





//This function make use of XIIC_Recv function to read data from the built in temprature sensor on the Nexys A7 100t
//using the i2c communction. Nothing Complex needed because the first two bytes have the Temp value data
//On Power on, the sensor points to the first register (data register) and it increment automatically
//Reading the datasheet, nothing extra needed for our system. 16 bit operation can be implemented but it is no use
//for our system

void GetTempValue(){
	int16_t temp_raw=0;  //to save the 13 bits we want here
	uint8_t temp_data[2]; // Buffer for two bytes. saving raw data to manipulate it

	XIic_Recv(IIC_DEVICE_ID, Temp_address,temp_data, 2, XIIC_STOP);
	  /*No need to do initialization or instances because this function is low level recieve function
	    The data acquired is raw . Meaning, it needs to be manipulated to get actual temperature.
	    From the datasheet: The sensor is in default mode - The register address by default point to the first temp
	    value (0x00 address) and it increment automatically to next address(No need to manually change the address) -
	    It works with 13 bit in default mode(one can go for 16 bit mode if desired) - How to change from raw data to
	    celsium temperature is in the datasheet.
	  */
		temp_raw=0;
	    temp_raw=temp_data[0];
	    temp_raw = temp_raw << 5;
	    temp_raw = temp_raw | (temp_data[1] >> 3);

	    temperature = temp_raw * 0.0625f; // To store the final temperature in C°

}







void EnableCaches() {
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_ICACHE
   Xil_ICacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
   Xil_DCacheEnable();
#endif
#endif
}

void DisableCaches() {
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_DCACHE
   Xil_DCacheDisable();
#endif
#ifdef XPAR_MICROBLAZE_USE_ICACHE
   Xil_ICacheDisable();
#endif
#endif
}
