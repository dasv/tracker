#ifndef _IMU_H
#define _IMU_H

#include "Arduino.h"
#include "types.h"

/********************************************************************
 *  
 * Variables
 * 
 * 
 ********************************************************************/
//extern Uart incSerial (&sercom3, 0, 1, SERCOM_RX_PAD_1, UART_TX_PAD_0); // Create the new UART instance assigning it to pin 0 and 1

String INCL_Read_XY(marineData *Data,  Uart *incSerial);
String INCL_Read_Temp(marineData *Data,  Uart *incSerial);

//void INCL_Resume();
//void INCL_Sleep();
void INCL_Print_All();

#endif
