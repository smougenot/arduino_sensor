#include <ESP8266WiFi.h>
#include <MQTT.h>


#define CLIENT_ID "client1"
#define TOPIC_IN  "/esp/1/cmd"
#define TOPIC_OUT "/esp/1/status"

// create MQTT object
MQTT myMqtt(CLIENT_ID, "192.168.1.12", 1883);

//
const char* ssid     = "MaisonSMT";
const char* password = "m3f13t01";

long lastMsg = 0;
char msg[50];
int value = 0;

//
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Connecting to MQTT server");  

  // setup callbacks
  myMqtt.onConnected(myConnectedCb);
  myMqtt.onDisconnected(myDisconnectedCb);
  myMqtt.onPublished(myPublishedCb);
  myMqtt.onData(myDataCb);
  
  Serial.println("connect mqtt...");
  myMqtt.connect();

  Serial.print("subscribe to topic...");
  Serial.println(TOPIC_IN);
  myMqtt.subscribe(TOPIC_IN);
  
  delay(10);
}

//
void loop() {

  int valueA0 = analogRead(A0);

  // publish value to topic
  publishStatus();
}

/**
 * send some data
 */
void publishStatus() {
  long now = millis();
  if (now - lastMsg > 2000) {

    // check
    if(!myMqtt.isConnected()){
      Serial.println("Not connected");
      myDisconnectedCb();
    }

    // prepare value
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "hello world #%ld", value);
    
    // publish value to topic
    Serial.print("publish to topic ");
    Serial.print(TOPIC_OUT);
    Serial.print(" : ");
    Serial.println(msg);
    String data = String(msg);
    String topic = String(TOPIC_OUT);
    boolean result = myMqtt.publish(topic, data);
  }
}

/*
 * 
 */
void myConnectedCb() {
  Serial.println("connected to MQTT server");
}

void myDisconnectedCb() {
  Serial.println("disconnected. try to reconnect...");
  delay(500);
  myMqtt.connect();
}

void myPublishedCb() {
  // too noisy
  Serial.println("published.");
}

void myDataCb(String& topic, String& data) {
  
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(data);
}



