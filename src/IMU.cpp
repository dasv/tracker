#include "types.h"
#include "IMU.h"

/************************************************** 
 * Incl Module
**************************************************/
String INCL_Read_XY(marineData *Data, Uart *incSerial)
{
	String a, val;
	incSerial->print("get-x&y");
	while (incSerial->available()<18) {} // Wait 'till there are 9 Bytes waiting
	for(int n=0; n<9; n++){
		a += char(incSerial->read());
	}

	if (a.toFloat() > Data->new_data.rollmax )
		Data->new_data.rollmax = a.toFloat();
	if (a.toFloat() < Data->new_data.rollmin )
		Data->new_data.rollmin = a.toFloat();
	val += "Roll: "; val += a;

	a = "";
	for(int n=0; n<8; n++){
		a += char(incSerial->read());
	}
	char(incSerial->read()); //Leemos el /n
	if (a.toFloat() > Data->new_data.pitchmax )
		Data->new_data.pitchmax = a.toFloat();
	if (a.toFloat() < Data->new_data.pitchmin )
		Data->new_data.pitchmin = a.toFloat();
	val += " Pitch: "; val += a;

	return val;
}

String INCL_Read_Temp(marineData *Data, Uart *incSerial)
{
	String a, val;
	incSerial->print("gettemp");
	while (incSerial->available()<6) {} // Wait 'till there are 9 Bytes waiting
	for(int n=0; n<5; n++){
		a += char(incSerial->read());
	}
	char(incSerial->read());
	Data->new_data.temp = a.toFloat();
	val += "Temp: "; val += a;
	return val;
}

//void INCL_Resume()
//{
//
//  
//}
//
//void INCL_Sleep()
//{
//
//  
//}

void INCL_Print_All(volatile marineData *Data)
{
	SerialUSB.print("Roll : "); SerialUSB.println(Data->new_data.rollmin);
	SerialUSB.print("Pitch: "); SerialUSB.println(Data->new_data.pitchmin);
	SerialUSB.print("Temp : "); SerialUSB.println(Data->new_data.temp);
}
