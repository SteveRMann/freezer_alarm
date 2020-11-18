#define SKETCH_NAME "freezerAlarm.ino"
#define SKETCH_VERSION "2.2"


/*
     Board: Lolin(Wemos) D1 R2 & Mini

     This project is for a freezer alarm.
     A DS18B20 will be inside the freezer and the Wemos D1 Mini with the OLED shield will be outside.
     Temperatures are also published on MQTT:
       fahrenheightTopic= freezer/temp/f
       centigradeTopic= freezer/temp/c
     V2.0 - working save
     V2.1 - Added OTA
     V2.1 - Added Ticker to flash red LED if the temperature rises above the alarmSetPoint

     Todo:
     Publish alarm status over MQTT
     Setup an MQTT cmd to set alarm alarmSetPoint.




*/


#include <OneWire.h>            // Driver for DS18S20, DS18B20, DS1822 Temperature Sensor.
#include "SSD1306Wire.h"        // Wire.h includes OLEDDisplay.h which contains the command definitions.

#define HOSTPREFIX "FRZR"
#include "ESP8266WiFi.h"        // Not needed if also using the Arduino OTA Library...
#include "D:\River Documents\Arduino\libraries\Kaywinnet.h"  \\ WiFi credentials
#include <PubSubClient.h>       // connect to a MQTT broker and publish/subscribe messages in topics.

#include <ArduinoOTA.h>
#include <Ticker.h>
Ticker REDFlipper;              // Ticker object

// setup_wifi vars
char macBuffer[24];             // Holds the last three digits of the MAC, in hex.
char hostNamePrefix[] = HOSTPREFIX;
char hostName[24];              // Holds hostNamePrefix + the last three bytes of the MAC address.


// Declare an object of class WiFiClient, which allows to establish a connection to a specific IP and port
// Declare an object of class PubSubClient, which receives as input of the constructor the previously defined WiFiClient.
// The constructor MUST be unique on the network.
WiFiClient frzrClient;
PubSubClient client(frzrClient);


#define NODENAME "freezer"                             // Give this node a name
const char *cmndTopic = NODENAME "/cmnd";              // Incoming commands, payload is a command.
const char *statusTopic = NODENAME "/status";
const char *fahrenheightTopic = NODENAME "/temp/f";
const char *centigradeTopic = NODENAME "/temp/c";
const char *connectName =  NODENAME "1";               // Must be unique on the network
const char *mqttServer = mqtt_server;                  // Local broker defined in Kaywinnet.h
const int mqttPort = 1883;


// Ceate instances of OneWire and SSD1360Wire
OneWire  ds(D4);                                      // ds18b20 on pin D4
SSD1306Wire  display(0x3c, D2, D1);                   // I2C address and pins for Wemos D1 Mini (0x3c,sda,scl, geometry)
//                                                    // Geometry =0 for 128x64 (default), =1 for 128x32
//SSD1306Wire display(0x3c, D2, D1, GEOMETRY_64_48 ); // WEMOS OLED shield


#define DEBUG true  //set to true for debug output, false for no debug ouput
#define Serial if(DEBUG)Serial

#define RED_LED D5
#define BLUE_LED D6
#define LEDOFF 0
#define LEDON 1

float alarmSetPoint = 23.0;         //Default value, To be set by MQTT.


// =================================== redToggle() ===================================
void redToggle() {
  digitalWrite(RED_LED, !digitalRead(RED_LED));     //Toggle the LED
}


// =================================== setup() ===================================
void setup(void) {
  beginSerial();
  setup_wifi();

  pinMode(BLUE_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  digitalWrite(BLUE_LED, LEDON);
  digitalWrite(RED_LED, LEDON);
  delay(1000);
  digitalWrite(BLUE_LED, LEDOFF);
  digitalWrite(RED_LED, LEDOFF);

  /*
    //Flip the RED LED every half second, for five seconds. (Demo)
    REDFlipper.attach(0.5, redToggle);
    delay(5000);
    REDFlipper.detach();
  */

  // Call the setServer method on the PubSubClient object, passing as first argument the
  // address and as second the port.
  client.setServer(mqttServer, mqttPort);
  mqttConnect();

  //Show the topics:
  Serial.print(F("statusTopic= "));
  Serial.println(statusTopic);
  Serial.print(F("fahrenheightTopic= "));
  Serial.println(fahrenheightTopic);
  Serial.print(F("centigradeTopic= "));
  Serial.println(centigradeTopic);
  Serial.println();



  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setBrightness(16);

  display.clear();                                      // clear the display
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, SKETCH_NAME);
  display.display();
  delay(3000);

}


// ==================================== loop() ====================================
void loop(void) {
  ArduinoOTA.handle();

  //Make sure we stay connected to the mqtt broker
  if (!client.connected()) {
    mqttConnect();
  }
  if (!client.loop()) {
    client.connect(connectName);
  }

  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;

  if (!ds.search(addr)) {
    ds.reset_search();
    delay(250);
    return;
  }


  if (OneWire::crc8(addr, 7) != addr[7])  {
    Serial.println("CRC is not valid!");
    return;
  }

  // the first ROM byte indicates which chip
  switch (addr[0])
  {
    case 0x10:
      type_s = 1;
      break;
    case 0x28:
      type_s = 0;
      break;
    case 0x22:
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);                // Start conversion
  delay(1000);
  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);

  // Read Scratchpad
  for ( i = 0; i < 9; i++) {
    data[i] = ds.read();
  }

  // Convert the data to actual temperature
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3;                // 9 bit resolution default
    if (data[7] == 0x10)
    {
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;                    // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3;               // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1;               // 11 bit res, 375 ms
  }

  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print(F("  Temperature = "));
  Serial.print(celsius);
  Serial.print(F(" Celsius, "));
  Serial.print(fahrenheit);
  Serial.println(F(" Fahrenheit"));
  Serial.println();

  // Send the temperature to the OLED display
  display.clear();                                      // clear the display
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);                    // font can be 10, 16 or 24
  display.drawString(0, 0, "Temperature");
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 16, String(fahrenheit) + " °F");
  display.drawString(0, 40, String(celsius) + " °C");
  display.display();

  // Publish the temperature on MQTT.
  // dtostrf(floatvar, StringLengthIncDecimalPoint, numVarsAfterDecimal, charbuf);
  char result[6];
  dtostrf(celsius, 4, 2, result);
  client.publish(centigradeTopic, result);

  //Alarm?
  if (celsius > alarmSetPoint) {
    REDFlipper.attach(0.5, redToggle);
  } else {
    REDFlipper.detach();
  }
}
