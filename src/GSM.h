#define TINY_GSM_MODEM_UBLOX

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

const char apn[]  = "YourAPN";
const char user[] = "";
const char pass[] = "";

const char server[] = "vsh.pp.ua";
const char resource[] = "/TinyGSM/logo.txt";
const int port = 80;

 //TinyGsm modem;