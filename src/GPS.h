#ifndef _GPS_H
#define _GPS_H

#include "Arduino.h"
#include "types.h"
#include "TinyGPS++.h"
//#include "rtcMod.h"

/********************************************************************
 *  
 * Variables
 * 
 * 
********************************************************************/

//TinyGPSPlus gps;

void GPS_Update(Uart *gpsSerie, TinyGPSPlus *gps, marineData *Data);
String GPS_time(Uart *gpsSerie, TinyGPSPlus * gps);
void GPS_Resume(Uart *gpsSerie, TinyGPSPlus *gps);

void GPS_Print_Serie(marineData *Data);
void GPS_Sleep();
void GPS_Wake();
void sendUBX(uint8_t *MSG, uint8_t len);
#endif
