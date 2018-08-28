#ifndef _RTC_aux_H
#define _RTC_aux_H

#include "Arduino.h"
#include "types.h"
#include <RTCZero.h> 

/********************************************************************
 *  
 * Variables
 * 
 * 
********************************************************************/

void RTC_Conf(RTCZero *rtc);
void RTC_Time_Print(RTCZero *rtc);
void print2digits(int number);
String RTC_Time(RTCZero *rtc);
String digits2string(int number);

#endif
