
/* ================================== mqttConnect() =================================
  Include at the top of the main ino file:
  #include <ESP8266WiFi.h>        // Connect (and reconnect) an ESP8266 to the a WiFi network.
  #include <PubSubClient.h>       // connect to a MQTT broker and publish/subscribe messages in topics.
  #include "C:\Users\steve\Documents\Arduino\libraries\Kaywinnet.h"          // Net credentials.


  // Declare an object of class WiFiClient, which allows to establish a connection to a specific IP and port
  // Declare an object of class PubSubClient, which receives as input of the constructor the previously defined WiFiClient.
  // The constructor MUST be unique on the network.
  WiFiClient pirClient;
  PubSubClient client(pirClient);

  #define NODENAME "motionSensor"                             // Give this node a name
  const char *cmndTopic = NODENAME "/cmnd";                   // Incoming commands, payload is a command.
  const char *connectName =  NODENAME "1";                    // Must be unique on the network
  const char *mqttServer = mqtt_server;                       // Local broker defined in Kaywinnet.h
  const int mqttPort = 1883;


  //----------
  //IN SETUP()
  // Call the setServer method on the PubSubClient object, passing as first argument the
  // address and as second the port.
  client.setServer(mqttServer, mqttPort);
  mqttConnect();

  //Show the topics:
  Serial.print(F("statusTopic= "));
  Serial.println(statusTopic);



  //----------
  //IN LOOP()
  //Make sure we stay connected to the mqtt broker
  if (!client.connected()) {
    mqttConnect();
  }
  if (!client.loop()) {
    client.connect(connectName);
  }


  //----------
  //Where you need to publish:
  client.publish(Topic, "on");

*/


void mqttConnect() {
  while (!client.connected()) {
    Serial.print(F("MQTT connection..."));
    if (client.connect(connectName)) {
      Serial.println(F("connected"));

      //Subscriptions:
      /*
        client.subscribe(cmndTopic);
        Serial.print(F("Subscribing to "));
        Serial.println(cmndTopic);
      */
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(client.state());
      Serial.println(F("- trying again in 5-seconds."));
      delay(5000);
    }
  }
}



// ==================================  mqtt callback ==================================
// This function is executed when some device publishes a message to a topic that this ESP8266 is subscribed to.
// The MQTT payload is the filename of the message to play when the phone is picked up.  The payload is case-sensitive.
//
void callback(String topic, byte * message, unsigned int length) {

  Serial.println();
  Serial.print(F("Message arrived on topic: "));
  Serial.println(topic);


  // Convert the character array to a string
  String messageString;
  for (int i = 0; i < length; i++) {
    messageString += (char)message[i];
  }
  messageString.trim();
  messageString.toUpperCase();          //Make the string upper-case


  Serial.print("messageString: ");
  Serial.print(messageString);
  Serial.println();



  if (topic == cmndTopic) {
    //Handle the command
  }

} //callback
