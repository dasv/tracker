#include <GSM.h>

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
HttpClient http(client, server, port);

void setupGSM(void){
    SerialAT.begin(115200);
    modem.init();
    //modem.simunlock("3225");
    SerialUSB.print("Waiting for cellular network...");
     if (!modem.waitForNetwork()) {
        SerialUSB.println(" fail");
        delay(10000);
        return;
    }
     SerialMon.println(" OK");

     if (!modem.gprsConnect(apn, user, pass)) {
        SerialMon.println(" fail");
        delay(10000);
        return;
    }
}

