#include "types.h"
#include "GPS.h"
#include "RTC_aux.h"


#define MAX_TRY 20
/************************************************** 
 * GPS Module
 **************************************************/

void GPS_Update(Uart *gpsSerie, TinyGPSPlus *gps, marineData *Data)
{
	int count = MAX_TRY;
	unsigned long start = millis();
	do
	{
		while (gpsSerie->available() > 0)
			gps->encode(gpsSerie->read());
	} while (millis() - start < 100);   // CLO para asegurar que le da tiempo a leer  se puede forzar hasta 50



// CLO comento las 6 filas de debajo

	//Para después de dormir el dispositivo.
	//while(gps->location.lat() == 0 && gps->location.lng() == 0 && count >= 0) {
	//	while (gpsSerie->available() > 0)
	//		gps->encode(gpsSerie->read());
	//	SerialUSB.println("GPS UPDATE FAIL");
	//	delay(300);
	//	count--;

	//}
		Data->ddmmaa = gps->date.value();
		Data->hhmmsscc = gps->time.value();
		Data->new_data.lat = gps->location.lat();
		Data->new_data.lon = gps->location.lng();
		Data->hour = gps->time.hour();
		Data->mins = gps->time.minute();
		Data->secs = gps->time.second();
		Data->cc = gps->time.centisecond();
		Data->day = gps->date.day();
		Data->month = gps->date.month();
		Data->year = gps->date.year();

		Data->new_data.heading = gps->course.deg();   // CLO lo pongo a grados
		Data->new_data.vel = gps->speed.knots();     // CLO lo pongo a nudos
		Data->new_data.sat = gps->satellites.value();       // CLO añado nº de satelites

}

String GPS_time(Uart *gpsSerie, TinyGPSPlus *gps) {
	String aux;

	aux += digits2string(gps->date.year());
	aux += digits2string(gps->date.month());
	aux += digits2string(gps->date.day());
	aux += digits2string(gps->time.hour());
	aux += digits2string(gps->time.minute());
	aux += digits2string(gps->time.second());
	aux += digits2string(gps->time.centisecond());

	return aux;


}

void GPS_Resume(Uart *gpsSerie, TinyGPSPlus *gps)
{

	while (Serial1.available() > 0)
		gps->encode(Serial1.read());

	//Para después de dormir el dispositivo.
	while(gps->time.hour() == 0) {
		while (gpsSerie->available() > 0)
			gps->encode(gpsSerie->read());
	}


}

void GPS_Wake(){
	uint8_t GPSon[] = {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00,0x09, 0x00, 0x17, 0x76};
	sendUBX(GPSon, sizeof(GPSon)/sizeof(uint8_t));
}

void GPS_Sleep()
{
	uint8_t GPSoff[] = {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00,0x08, 0x00, 0x16, 0x74};
	sendUBX(GPSoff, sizeof(GPSoff)/sizeof(uint8_t));
}

// Send a byte array of UBX protocol to the GPS
void sendUBX(Uart *gpsSerie, uint8_t *MSG, uint8_t len) {
	for (int i=0; i<len; i++) {
		gpsSerie->write(MSG[i]);
	}
}

String GPS_Print(marineData Data)
{
	String val;
	val += " Nºsat: "; val += String(Data.new_data.sat,5); val += " ";  //CLO añadimos el nº de satelites usados
	val += "Lat: "; val += String(Data.new_data.lat,8); val += " ";
	val += "Long: "; val += String(Data.new_data.lon,8); val += " ";
	val += "Vel: "; val += String(Data.new_data.vel,4); val += " ";
	val += "Heading: "; val += String(Data.new_data.heading,4); val += " "; //4 decimales de precisión para qué? El GPS sólo da 2
	//val += "Temp: "; val += Data.data.temp; val += " ";
	val += "Volt: "; val += Data.new_data.vbatt; //val += " ";
	return val;
}

void GPS_Print_Serie(marineData *Data)
{
	SerialUSB.print("GPS.LAT: ");  Serial.println(Data->new_data.lat);
	SerialUSB.print("GPS.Lon: ");  Serial.println(Data->new_data.lon);
	SerialUSB.print("GPS.DAY: ");  Serial.print(Data->day);
	SerialUSB.print(" GPS.MONTH: ");  Serial.print(Data->month);
	SerialUSB.print(" GPS.YEAR: ");  Serial.println(Data->year);

	SerialUSB.print("GPS.HOUR: ");  Serial.print(Data->hour);
	SerialUSB.print(" GPS.MINS: ");  Serial.print(Data->mins);
	SerialUSB.print(" GPS.SECS: ");  Serial.print(Data->secs);
	SerialUSB.print(" GPS.mS: ");  Serial.println(Data->cc);

	SerialUSB.print("GPS.DDMMAA: ");  Serial.println(Data->ddmmaa);
	SerialUSB.print("GPS.HHMMSSCC: ");  Serial.println(Data->hhmmsscc);
}


