//setup_wifi
//============================= Connect the ESP to the router =============================
//Connect to WiFi network so we can reach the MQTT broker and publish messages to topics.

/*
  Make sure you include at the start of the sketch:
  #define HOSTPREFIX "CGM-"
  #include "ESP8266WiFi.h"   //Not needed if also using the Arduino OTA Library...
  #include "D:\River Documents\Arduino\libraries\Kaywinnet.h"  \\ WiFi credentials

  If using the OTA Library, put these at the start of the sketch.
  //setup_wifi vars
  char macBuffer[24];       //Holds the last three digits of the MAC, in hex.
  char hostNamePrefix[] = HOSTPREFIX;
  char hostName[24];        //Holds hostNamePrefix + the last three bytes of the MAC address.
*/

void setup_wifi() {
#ifndef Kaywinnet
#include <kaywinnet.h>
#endif
  byte mac[6];                     ////the MAC address of your Wifi shield

  Serial.println(F("\n"));
  Serial.print(F("Connecting to "));
  Serial.println(IOT_SSID);


  WiFi.mode(WIFI_STA);
  //WiFi.begin(MYSSID, MYPASSWORD);
  WiFi.begin(IOT_SSID, IOT_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(WiFi.status());
  }
  Serial.println(F("\nWiFi connected, "));
  Serial.print(F("MAC Address: "));
  Serial.println(WiFi.macAddress());
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());


  //Get the last three numbers of the mac address.
  //"4C:11:AE:0D:83:86" becomes "0D8386" in macBuffer.
  WiFi.macAddress(mac);
  snprintf(macBuffer, sizeof(macBuffer), "%02X%02X%02X", mac[3], mac[4], mac[5]);

  //Build hostName from hostNamePrefix + last three bytes of the MAC address.
  strcpy(hostName, hostNamePrefix);
  strcat(hostName, "-");
  strcat(hostName, macBuffer);

  Serial.print(F("hostName = \""));
  Serial.print(hostName);
  Serial.println(F("\""));

}



//=============================== macToStr ===============================
/* Make a string from the MAC address
  Useage:
  uint8_t mac[6];
  WiFi.macAddress(mac);
  Serial.print(F("MAC Address: "));
  Serial.println(macToStr(mac));
*/

String macToStr(const uint8_t* mac) {
  String result;
  for (int i = 0; i < 6; ++i) {
    if (mac[i] < 10) {
      result += '0';
      result += String(mac[i], 16);
    }
    else {
      result += String(mac[i], 16);
    }
    if (i < 5)  //Add a colon between hex duplets
      result += ':';
  }
  return result;
}


/*
  //============================= Reconnect to the MQTT Broker =============================
  void reconnect()
  {
  while (!client.connected()) {
    Serial.println(F("\nAttempting MQTT connection..."));

    if (client.connect(hostName))  {            //hostName is built in setup_wifi()
      Serial.print(F("connected, "));
  //     Serial.println(F("Subscribed to:"));
  //     Serial.println(cmdTopic);

  //     client.subscribe(unlockTopic);
    }
    else {
      Serial.print(F("failed, rc= "));
      Serial.print(client.state());
      Serial.println(F(", trying again in 5 seconds."));
      delay(5000);
    }
  }
  }
*/
