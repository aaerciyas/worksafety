
#ifndef __CC3200R1M1RGC__
// Do not include SPI for CC3200 LaunchPad
#include <SPI.h>
#endif
#include <WiFi.h>
#include "sha256.h"
#include "Base64.h"
#include <BMA222.h>
#include <Wire.h>

//#define USE_USCI_B1 
//Reference : http://www.ti.com.cn/cn/lit/ug/swru372b/swru372b.pdf / http://blog.gaku.net/cc3200-launchpad/
BMA222 mySensor;
void printFloat(float value, int places);

// START: WiFi settings
char SSID[] = "AndroidA";
char PASSWORD[] = "izql4785";
// END: WiFi settings

// START: Azure IoT Hub settings

//Azure IoT Hub connection string
String IoTHubConnectionString = "HostName=workSafetyiot.azure-devices.net;DeviceId=cc3200device;SharedAccessKey=ckUZyY24LnBguks9gw1w0XKjGHg61sd1SNGkrda3V30=";

boolean isConnectedToIoTHub = false;
const char* IOT_HUB_END_POINT = "/messages/events?api-version=2016-02-03";
char* HOST;
char* DEVICE_ID;
char* KEY;
String strSasToken;
// END: Azure IoT Hub settings

String dataToSend; //String variable which will be containing the JSON data to send

//define equipments
const int buttonBaret = 17;     // the number of the pushbutton pin
const int buttonKemer = 19;

//pin status
int baretDurum = 0;
int kemerDurum = 0;
int baretdurumeski=0;
int kemerdurumeski=0;
int durum=0;

WiFiClient client;

void setup() {
     
pinMode(buttonBaret, INPUT_PULLUP);   
pinMode(buttonKemer,INPUT_PULLUP);

    Serial.begin(115200);
  
    // attempt to connect to Wifi network:
    Serial.print("Attempting to connect to Network named: ");
    // print the network name (SSID);
    Serial.println(SSID); 
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    WiFi.begin(SSID,PASSWORD);
    while ( WiFi.status() != WL_CONNECTED) {
      // print dots while we wait to connect
      Serial.print(".");
      delay(300);
  }
  
  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");
  
  while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nIP Address obtained");
  printWifiStatus();
  
  HOST = (char*)GetStringValue(splitStringByIndex(splitStringByIndex(IoTHubConnectionString, ';', 0), '=', 1));
  DEVICE_ID = (char*)GetStringValue(splitStringByIndex(splitStringByIndex(IoTHubConnectionString, ';', 1), '=', 1));
  KEY = (char*)GetStringValue(splitStringByIndex(splitStringByIndex(IoTHubConnectionString, ';', 2), '=', 1));
  
  strSasToken = createIotHubSasToken((char*)KEY, urlEncode(HOST) + urlEncode("/devices/") +  urlEncode(DEVICE_ID) );

  Serial.println("\nStarting connection to Azure IoT Hub...");
  isConnectedToIoTHub = client.sslConnect(HOST, 443);

  while(!isConnectedToIoTHub){    
    Serial.print(".");
    delay(300);
  }
  
  Serial.println("Setup is finished ...");
baretdurumeski=digitalRead(buttonBaret);
kemerdurumeski=digitalRead(buttonKemer);

  mySensor.begin();
  uint8_t chipID = mySensor.chipID();
  Serial.print("chipID: ");
  Serial.println(chipID);
}

void loop() {
  delay(2000);
  //buton dinleme
  baretDurum = digitalRead(buttonBaret);
  kemerDurum = digitalRead(buttonKemer);
  int8_t data = mySensor.readYData();
  Serial.print("Y: ");
  Serial.print(data);
  if((data<30) && (data>-30)){
  delay(5000);
  int olcum = mySensor.readYData();
  if((olcum<30) && (olcum>-30)){
  Serial.print("Tehlike durumu\n");
  durum=1;}
else {
 durum=0;
  } 
 }
  if (baretDurum==baretdurumeski&&kemerDurum==kemerdurumeski&&durum==0){
     Serial.println("degisim yok\n");
     Serial.println("Buton Durumlari : " + (String)baretDurum + " " + (String)kemerDurum+ " " + (String)durum);
  }
  
  else {
    
  dataToSend = "{\"Baret\":" + (String)baretDurum + ", \"Kemer\":" + (String)!kemerDurum + ", \"Durum\":" + (String)durum + "}";
  Serial.println("Buton Durumlari : " + (String)baretDurum + " " + (String)kemerDurum+ " " + (String)durum);
  // END: JSON data to send
  
  Serial.println("\Sending data to Azure IoT Hub...");

  // if you get a connection, report back via serial:
  if (isConnectedToIoTHub) {
    // https://msdn.microsoft.com/en-us/library/azure/dn790664.aspx
    
    String request = String("POST ") + "/devices/" + (String)DEVICE_ID + (String)IOT_HUB_END_POINT + " HTTP/1.1\r\n" +
                     "Host: " + HOST + "\r\n" +
                     "Authorization: SharedAccessSignature " + strSasToken + "\r\n" + 
                     "Content-Length: " + dataToSend.length() + "\r\n\r\n" +
                     dataToSend;
    client.print(request);
   
  }
else {
  Serial.println("\Baglanti kesildi");
}
baretdurumeski  =  baretDurum;
kemerdurumeski  =  kemerDurum;
}
}
