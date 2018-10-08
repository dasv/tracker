#include <Arduino.h>
#include "wiring_private.h"
#include "types.h"
#include "IMU.h"
#include "GPS.h"
#include <SDFat.h>
#include <TinyGPS++.h>
#include <IridiumSBD.h>
#include <RTCZero.h>
#include "packing.h"


#define error(msg) sd.errorHalt(F(msg))

#define SLEEP_PIN 6

#define SERIAL_EN true

//#define gpsSerial Serial1

#define GPS_BASE_NAME  "GPS"
#define INC_BASE_NAME  "INC"
#define SBD_BASE_NAME  "SBD"


/**
 * DECLARACIONES GLOBALES
 */

//puertos serie
Uart incSerial (&sercom3, 0, 1, SERCOM_RX_PAD_1, UART_TX_PAD_0); // Create the new UART instance assigning it to pin 0 and 1
Uart modemSerial (&sercom4, 4, 5, SERCOM_RX_PAD_3, UART_TX_PAD_2); // Create the new UART instance assigning it to pin 6 and 7

//pines sueltos
static int SolarPin = 8;

//periféricos
IridiumSBD isbd(modemSerial, SLEEP_PIN);
TinyGPSPlus gps;
RTCZero rtc;

//tiempos de ejecucion
uint32_t tiempo_corto = 0;
uint32_t tiempo_largo = 0;
uint32_t tiempo_act_gps = 0;
uint32_t tiempo_log_gps = 0;
uint32_t tiempo_imu = 0; 
uint32_t tiempo_log_imu = 0;

//struct para guardar datos
marineData Data;

//nombres archivos
char GPSfileName[10] = GPS_BASE_NAME;
char IMUfileName[10] = INC_BASE_NAME;
char SBDfileName[10] = SBD_BASE_NAME;
//objetos SD
SdFat sd;
const uint8_t chipSelect = SDCARD_SS_PIN;
SdFile file;
//buffer RX Iridium
uint8_t SBDbuffer[200];
static bool messageSent = false;

//contador aperturas fichero IMU
int imu_counter = 0;



/**
 * FUNCIONES
 */

//Init Data

void init_data(void) {

	//Value Default
	Data.new_data.pitchmax = -100000;
	Data.new_data.rollmax = -100000;
	Data.new_data.pitchmin = 100000;
	Data.new_data.rollmin = 100000;

	Data.old_data.heading = 0;
	Data.old_data.lat = 0;
	Data.old_data.lon = 0;
	Data.old_data.pitchmax = -100000;
	Data.old_data.pitchmin = 100000;
	Data.old_data.rollmax = -100000;
	Data.old_data.rollmin = 100000;
	Data.old_data.temp = 0;
	Data.old_data.vbatt = 0;
	Data.old_data.vel = 0;
    Data.old_data.sat = 0;
}

//scheduler para tiempos cortos, intervalos en MILISEGUNDOS
uint32_t scheduler_helper(void (*fptr)(), uint32_t last_time, uint32_t interval) {
	uint32_t now = millis();
	if ((now - last_time) > interval){
		fptr();
		last_time = now;
	}

	return last_time;
}
//scheduler para tiempos largos, argumentos en SEGUNDOS
uint32_t scheduler_helper_long (void (*fptr)(), uint32_t last_time, uint32_t interval) {
    uint32_t now = rtc.getEpoch();
	if ((now - last_time) > interval){
		fptr();
		last_time = now;
	}

	return last_time;
}

void initSdFile(char * BASE_NAME, SdFile file) {
        const uint8_t BASE_NAME_SIZE = sizeof(BASE_NAME) - 1;
        char fileName[13];
        strcpy(fileName, BASE_NAME);
        strcat(fileName, "00.DAT");

        if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) {
            sd.initErrorHalt();
        }

        // Find an unused file name.
        if (BASE_NAME_SIZE > 6) {
        error("FILE_BASE_NAME too long");
        }

        while (sd.exists(fileName)) {
            if (fileName[BASE_NAME_SIZE + 1] != '9') {
                fileName[BASE_NAME_SIZE + 1]++;
            } else if (fileName[BASE_NAME_SIZE] != '9') {
                fileName[BASE_NAME_SIZE + 1] = '0';
                fileName[BASE_NAME_SIZE]++;
            } else {
                error("Can't create file name");
            }
        }

        //REVISAR: es conveniente dejar el fichero abierto permanentemente o abrirlo cada vez que se escribe? Mover a logData en tal caso.
        if (!file.open(fileName, O_CREAT | O_WRITE | O_EXCL)) {
            error("file.open");
        }

        BASE_NAME = fileName; //guardamos el nombre con los dígitos

        file.close();

}

void logData(String val, char* fileName) {
    file.close();
    file.open(fileName, O_WRITE);
    file.println(val);
    file.close();
}

String GPS_Print(void)
{
	String val;
	val += " Nºsat: "; val += String(Data.new_data.sat,5); val += " ";  //CLO añadimos el nº de satelites usados
	val += "Lat: "; val += String(Data.new_data.lat,8); val += " ";
	val += "Long: "; val += String(Data.new_data.lon,8); val += " ";
	val += "Vel: "; val += String(Data.new_data.vel,4); val += " ";
	val += "Heading: "; val += String(Data.new_data.heading,4); val += " ";
	//val += "Temp: "; val += Data.data.temp; val += " ";
	val += "Volt: "; val += Data.new_data.vbatt; //val += " ";
    //SerialUSB.println(val);
	return val;
}

void IMUdata(void) {
    String val;
    //val += RTC_Time(&rtc); val += ": ";
    val += GPS_time(&Serial1, &gps); val += ": "; //debug cambiamos tiempo rtc por consulta continua al gps


    val += INCL_Read_XY(&Data, &incSerial); val += " ";
    val += INCL_Read_Temp(&Data, &incSerial);


    // CLO para que imprima en el puerto serie y en la SD datos del GPS
    //GPS_Update(&gpsSerial, &gps, &Data);           // en esta funcion he metido mas valores, satelites, he modificado la velcoidad, etc
    val += GPS_Print();

    //logData(val, IMUfileName);   

    //las otras funciones siempre cierran el fichero cada vez que escriben, si está abierto alguno tiene que ser el de la IMU
    if (file.isOpen()){
        file.println(val);
        imu_counter++;
    }
    else {
        file.open(IMUfileName, O_WRITE);
        file.println(val);
        imu_counter = 0;
    }
    if (imu_counter >= 10) {
        file.close();
        imu_counter = 0;
    }

    	if (SERIAL_EN)
        SerialUSB.println(val);
    
   // return val;
}

void GPS_data(void) {
	
	String val;

    if (SERIAL_EN) {
        SerialUSB.println("");
        SerialUSB.println("GPS UPDATE START");
    }

    // CLO para que imprima tambien GPS en el puerto serie
    //GPS_Update(&gpsSerial, &gps, &Data);
    //val += RTC_Time(&rtc); val += ": ";
    val += GPS_time(&Serial1, &gps); val += ": "; //debug cambiamos tiempo rtc por consulta continua al gps

    val += GPS_Print();

    logData(val, GPSfileName);

}


void batteryVoltage(void) {
    int sensorValue = analogRead(A1);
    float voltage = sensorValue / 63.49;

    // CLO para descoenctar los paneles en caso de sobrecarga de bateria
    if (voltage > 13.5) {
        digitalWrite(SolarPin, LOW);
    }
    else {
        digitalWrite(SolarPin, HIGH);
    }

    Data.new_data.vbatt = voltage;
}

void func_corto(void){
    SerialUSB.println("corto");
}

void func_largo(void){
    SerialUSB.println("largo");
}

void send_SBD(void) {
    packer(Data);
    //TODO: comprobar si los datos no son 0 y hay un new y old disponibles para enviar
    int err;
    isbd.begin();
  
    // Read/Write the first time or if there are any remaining messages
    if (!messageSent || isbd.getWaitingMessageCount() > 0) {
        size_t bufferSize = sizeof(SBDbuffer);

        // First time through send+receive; subsequent loops receive only
        if (!messageSent)
            err = isbd.sendReceiveSBDBinary(p, package_size, SBDbuffer, bufferSize);
        else
            err = isbd.sendReceiveSBDText(NULL, SBDbuffer, bufferSize);
            
        if (err != ISBD_SUCCESS) {
            SerialUSB.print("sendReceiveSBD* failed: error ");
            SerialUSB.println(err);
        }
        else {  // success! 
            messageSent = true;
            SerialUSB.print("Inbound buffer size is ");
            SerialUSB.println(bufferSize);

            for (uint16_t i=0; i<bufferSize; ++i) {
                SerialUSB.print(SBDbuffer[i], HEX);
                if (isprint(SBDbuffer[i])) {
                    SerialUSB.print("(");
                    SerialUSB.write(SBDbuffer[i]);
                    SerialUSB.print(")");
                }
                SerialUSB.print(" ");
            }

            SerialUSB.println();
            SerialUSB.print("Messages remaining to be retrieved: ");
            SerialUSB.println(isbd.getWaitingMessageCount());
        }        
    }
    isbd.sleep();
}

void send_SBD_prueba(void) {
    uint8_t mensaje[13] = "SYSTEM READY";
    //TODO: comprobar si los datos no son 0 y hay un new y old disponibles para enviar
    int err;
    isbd.begin();
  
    // Read/Write the first time or if there are any remaining messages
    if (!messageSent || isbd.getWaitingMessageCount() > 0) {
        size_t bufferSize = sizeof(SBDbuffer);

        // First time through send+receive; subsequent loops receive only
        if (!messageSent)
            err = isbd.sendReceiveSBDBinary(mensaje, sizeof(mensaje), SBDbuffer, bufferSize);
        else
            err = isbd.sendReceiveSBDText(NULL, SBDbuffer, bufferSize);
            
        if (err != ISBD_SUCCESS) {
            SerialUSB.print("sendReceiveSBD* failed: error ");
            SerialUSB.println(err);
        }
        else {  // success! 
            messageSent = true;
            SerialUSB.print("Inbound buffer size is ");
            SerialUSB.println(bufferSize);

            for (uint16_t i=0; i<bufferSize; ++i) {
                SerialUSB.print(SBDbuffer[i], HEX);
                if (isprint(SBDbuffer[i])) {
                    SerialUSB.print("(");
                    SerialUSB.write(SBDbuffer[i]);
                    SerialUSB.print(")");
                }
                SerialUSB.print(" ");
            }

            SerialUSB.println();
            SerialUSB.print("Messages remaining to be retrieved: ");
            SerialUSB.println(isbd.getWaitingMessageCount());
        }        
    }
    isbd.sleep();

    //parseSBD(SBDbuffer);
}

/**
 * Interpreta los mensajes de control
 * Formato de mensaje: AxxTyyIzzzz
 * Donde xx es el tiempo en HORAS que dormirá el tracker. Si es 00 no dormirá. Rango 00-99
 * yy es la frecuencia en MINUTOS en la que se enviarán mensajes, rango 00-99. La frecuencia de adquisición de datos de GPS es la mitad de esta frecuencia.
 * zzzz es la frecuencia en MILISEGUNDOS en la que se actualizará la IMU.
 * Para yy, zzzz, dejarlos a 00 o 0000 significará que no se cambia el valor aactual.
 */

void parse_SBD(uint8_t * incoming) {
    uint8_t mins_apagado;
    uint8_t freq_trama;
    uint16_t freq_imu;

    if(incoming[0] == 'A' && incoming[3] == 'T' && incoming[6] == 'I') {
        mins_apagado = incoming[2] - '0'; //convertimos char a int, ya tiene el cast a int implicito
        mins_apagado += (incoming[1] - '0') * 10;
        if(mins_apagado > 99)
            mins_apagado = 99; //comprobacion por paranoia
        freq_trama = incoming[5] - '0';
        freq_trama += (incoming[4] - '0') * 10;
        if (freq_trama > 99)
            freq_trama = 99;
        freq_imu = incoming[10] - '0';
        freq_imu += (incoming[9] - '0') * 10;
        freq_imu += (incoming[8] - '0') * 100;
        freq_imu += (incoming[7] - '0') * 1000;
        if (freq_imu > 9999)
            freq_imu = 10000; //por dejarlo redondo nada más

        SerialUSB.print('A');
        SerialUSB.print(mins_apagado);
        SerialUSB.print('T');
        SerialUSB.print(freq_trama);
        SerialUSB.print('I');
        SerialUSB.println(freq_imu);
    } else {
        SerialUSB.println("received invalid configuration string format");
    }
}

void CycleData(void) {

    if (SERIAL_EN)
        SerialUSB.println("DATA UPDATE START");
    Data.old_data.heading = Data.new_data.heading;
    Data.old_data.lat = Data.new_data.lat;
    Data.old_data.lon = Data.new_data.lon;
    Data.old_data.pitchmax = Data.new_data.pitchmax;
    Data.old_data.pitchmin = Data.new_data.pitchmin;
    Data.old_data.rollmax = Data.new_data.rollmax;
    Data.old_data.rollmin = Data.new_data.rollmin;
    Data.old_data.temp = Data.new_data.temp;
    Data.old_data.vbatt = Data.new_data.vbatt;
    Data.old_data.vel = Data.new_data.vel;

    Data.new_data.heading = 0;
    Data.new_data.lat = 0;
    Data.new_data.lon = 0;
    Data.new_data.pitchmax = -100000;
    Data.new_data.pitchmin = 100000;
    Data.new_data.rollmax = -100000;
    Data.new_data.rollmin = 100000;
    Data.new_data.temp = 0;
    Data.new_data.vbatt = 0;
    Data.new_data.vel = 0;

		//sendrdy++;
}

void PrintData(void) {

		SerialUSB.println("");
		SerialUSB.println("");
    	SerialUSB.println("DATA: ");
    	SerialUSB.print(Data.ddmmaa);
    	SerialUSB.print(Data.hhmmsscc);
    	SerialUSB.print(" Lat=");
    	SerialUSB.print(Data.new_data.lat,6);
    	SerialUSB.print(" Lon=");
    	SerialUSB.print(Data.new_data.lon,6);
    	SerialUSB.print(" PitchMax=");
    	SerialUSB.print(Data.new_data.pitchmax);
    	SerialUSB.print(" PitchMin=");
    	SerialUSB.print(Data.new_data.pitchmin);
    	SerialUSB.print(" RollMax=");
    	SerialUSB.print(Data.new_data.rollmax);
    	SerialUSB.print(" RollMin=");
    	SerialUSB.print(Data.new_data.rollmin);

    	SerialUSB.print(" Temp=");
    	SerialUSB.print(Data.new_data.temp);
    	SerialUSB.print(" Vol=");
    	SerialUSB.print(Data.new_data.vbatt);
    	SerialUSB.print(" Vel=");
    	SerialUSB.print(Data.new_data.vel);
    	SerialUSB.print(" Heading=");
    	SerialUSB.print(Data.new_data.heading);

    	SerialUSB.print(" LatOLD=");
    	SerialUSB.print(Data.old_data.lat,6);
    	SerialUSB.print(" LonOLD=");
    	SerialUSB.print(Data.old_data.lon,6);
    	SerialUSB.print(" PitchMaxOLD=");
    	SerialUSB.print(Data.old_data.pitchmax);
    	SerialUSB.print(" PitchMinOLD=");
    	SerialUSB.print(Data.old_data.pitchmin);
    	SerialUSB.print(" RollMaxOLD=");
    	SerialUSB.print(Data.old_data.rollmax);
    	SerialUSB.print(" RollMinOLD=");
    	SerialUSB.print(Data.old_data.rollmin);

    	SerialUSB.print(" TempOLD=");
    	SerialUSB.print(Data.old_data.temp);
    	SerialUSB.print(" VoltOLD=");
    	SerialUSB.print(Data.old_data.vbatt);
    	SerialUSB.print(" VelOLD=");
    	SerialUSB.print(Data.old_data.vel);
    	SerialUSB.print(" HeadingOLD=");
    	SerialUSB.print(Data.old_data.heading);

    	SerialUSB.println(" ");
		SerialUSB.println(" ");
}



void GPS_act_aux(void){
    GPS_Update(&Serial1, &gps, &Data);
}

void setup() {
    rtc.begin();
   	pinMode(SolarPin, OUTPUT);          // sets the digital pin as outpu  CLO   se usa para activar un reley que corta la corriente de los paneles si la tension es mayor de un valor, unos 13V, para no sobrecargar la bateria.
    SerialUSB.begin(9600);
    delay(10000);
    SerialUSB.println("begin setup");

    init_data();

    //inicialización archivos SD
    initSdFile(GPSfileName, file);
    initSdFile(IMUfileName, file);
    initSdFile(SBDfileName, file);

    //inicialización puertos serie
    Serial1.begin(9600); 

    incSerial.begin(38400);
    pinPeripheral(0, PIO_SERCOM); //Assign RX function to pin 1
	pinPeripheral(1, PIO_SERCOM); //Assign TX function to pin 0

    modemSerial.begin(19200);
    pinPeripheral(4, PIO_SERCOM_ALT); //Assign RX function to pin 2
	pinPeripheral(5, PIO_SERCOM_ALT); //Assign TX function to pin 3
}


void loop() {
    //tiempo_corto = scheduler_helper(func_corto, tiempo_corto, 1000);
    //tiempo_largo = scheduler_helper_long(send_SBD_prueba, tiempo_largo, 30);
    /*SerialUSB.println("loop"); 
    send_SBD_prueba();
    parse_SBD(SBDbuffer);
    delay(10000);*/

    tiempo_act_gps = scheduler_helper(GPS_act_aux, tiempo_act_gps, 100);
    tiempo_log_gps = scheduler_helper(GPS_data, tiempo_log_gps, 1000);
    tiempo_imu = scheduler_helper(IMUdata, tiempo_imu, 250);
    //tiempo_log_imu = scheduler_helper()
    tiempo_largo = scheduler_helper_long(CycleData, tiempo_largo, 300);


}

/********************************************************************
 *
 * Interrupt
 *
 *
********************************************************************/

// Attach the interrupt handler to the SERCOM
void SERCOM3_Handler()
{
    incSerial.IrqHandler();
}

void SERCOM4_Handler()
{
    modemSerial.IrqHandler();
}