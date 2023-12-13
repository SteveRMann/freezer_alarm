//=================================== setup() ===================================
void setup() {
  pinMode(BLUE_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  //Turn on the LEDS while starting up.
  digitalWrite(BLUE_LED, LEDON);
  digitalWrite(RED_LED, LEDON);

  beginSerial();
  setup_wifi();
  start_OTA();


  /*
    //Flip the RED LED every half second, for five seconds. (Demo)
    REDFlipper.attach(0.5, redToggle);
    delay(5000);
    REDFlipper.detach();
  */

  //Call the setServer method on the PubSubClient object, passing as first argument the
  //address and as second the port.
  client.setServer(mqttServer, mqttPort);
  mqttConnect();

  //Show the topics:
  Serial.println();
  Serial.print(F("MQTT Server IP= "));
  Serial.println(mqttServer);
  Serial.print(F("alarmTopic= "));
  Serial.println(alarmTopic);
  Serial.print(F("fahrenheightTopic= "));
  Serial.println(fahrenheightTopic);
  Serial.print(F("centigradeTopic= "));
  Serial.println(centigradeTopic);
  Serial.print(F("macAddressTopic= "));
  Serial.println(macAddressTopic);
  Serial.println();

  client.publish(macAddressTopic, macAddress);

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setBrightness(16);

  display.clear();                                      //clear the display
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, SKETCH_NAME);
  display.display();

  //Turn off the LEDS
  digitalWrite(BLUE_LED, LEDOFF);
  digitalWrite(RED_LED, LEDOFF);
}
