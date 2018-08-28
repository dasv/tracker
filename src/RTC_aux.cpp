#include "types.h"
#include "RTC_aux.h"

/************************************************** 
 * RTC Module
**************************************************/
void RTC_Conf(RTCZero *rtc)
{
  // RTC 
  rtc->begin(); // initialize RTC 24H format

  //rtc->setAlarmTime(16, 0, 10);
  //rtc->enableAlarm(rtc->MATCH_HHMMSS);
 
  //rtc->attachInterrupt(alarmMatch);
  
}

void RTC_Time_Print(RTCZero *rtc)
{
  // Print date...
  print2digits(rtc->getDay());
  SerialUSB.print("/");
  print2digits(rtc->getMonth());
  SerialUSB.print("/");
  print2digits(rtc->getYear());
  SerialUSB.print(" ");
  
  // ...and time
  print2digits(rtc->getHours());
  SerialUSB.print(":");
  print2digits(rtc->getMinutes());
  SerialUSB.print(":");
  print2digits(rtc->getSeconds());
  
  SerialUSB.println();

}


void print2digits(int number) {
  if (number < 10) {
    SerialUSB.print("0"); // print a 0 before if the number is < than 10
  }
  SerialUSB.print(number);
}

String RTC_Time(RTCZero *rtc)
{
  String aux;
  // Print date...
  aux += digits2string(rtc->getYear());
  aux += digits2string(rtc->getMonth());
  aux += digits2string(rtc->getDay());
  aux += digits2string(rtc->getHours());
  aux += digits2string(rtc->getMinutes());
  aux += digits2string(rtc->getSeconds());
  return aux;

}

String digits2string(int number) 
{
  String A;
  A += String((number/10)); // print a 0 before if the number is < than 10
  A += String((number % 10));
  return A;
}

