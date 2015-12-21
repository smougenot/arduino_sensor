// BMP180
#include <Wire.h>
#include <Adafruit_BMP085.h>
// Display
#include <TM1637Display.h>
// send messages
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define PRJ_VERSION 2

// wait time (ms)
#define LOOP_WAIT 60 * 1000

// I2C pins
#define IC_CLK  14
#define IC_DATA 12

// display pins
#define CLK   2
#define DATA  0

// Display
TM1637Display display(CLK, DATA);

//Sensor

// Connect VCC of the BMP085 sensor to 3.3V (NOT 5.0V!)
// Connect GND to Ground
// Connect SCL to i2c clock - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 5 esp8266 -> #5
// Connect SDA to i2c data - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 4  esp8266 -> #4
// EOC is not used, it signifies an end of conversion
// XCLR is a reset pin, also not used here

Adafruit_BMP085 bmp;

// MQTT
#define CLIENT_ID     "1"
const char* mqtt_server = "192.168.1.17";
const char* topicCmd    = "/esp/1/cmd";
const char* topicStatus = "/esp/1/status";
const char* clientId    = "ESP8266Client1";
// network.
const char* ssid = "MaisonSMT";
const char* password = "m3f13t01";

WiFiClient espClient;
PubSubClient client(espClient);

// Datas
float temperature = -1;
float altitude = -1;
int pressure = -1;
short loopCnt = 0;
// timing
long lastCheck = 0;

// msg
char msg[50];
char topic[50];

void info(){
    Serial.println("sensor to mqtt");
    Serial.print("VERSION: ");
    Serial.println(PRJ_VERSION);
    Serial.println();
    Serial.print("Wifi: ");
    Serial.println(ssid);
    Serial.print("mqtt: ");
    Serial.println(mqtt_server);
    Serial.print("topics: ");
    Serial.print(topicStatus);
    Serial.print(" , ");
    Serial.println(topicCmd);
    Serial.print("wait (ms)");
    Serial.println(LOOP_WAIT);  
}

void setup()
{
    Serial.begin(115200);
    info();

    // network
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    reconnect();
    
    //
    // Display
    //
    display.setBrightness(0x0f);
    // All segments on
    displayAllOn();

    // Sensor
    Serial.println("Setting sensor");
    Wire.pins(IC_DATA, IC_CLK);

    if (!bmp.begin()) {
      Serial.println("Could not find a valid BMP180 sensor, check wiring!");
      // loop will force reboot
      while (1) {}
    }
    Serial.println("Sensor ready");

    // première mesure
    Serial.println("Lancement de la première mesure");  
    displayAllOff();
    work();
    
    Serial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C)\tTime (us)");
}


void loop()
{
  
  // check incomming/connection
  checkClient();

  long now = millis();
  if (now - lastCheck > LOOP_WAIT) {
    lastCheck = now;
    work();
  }
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void checkClient() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void reconnect() {
  Serial.println("reconnect");
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientId)) {
      Serial.print("connected with id : ");
      Serial.println(clientId);
      // Once connected, publish an announcement...
      client.publish(topicStatus, "hello world");
      Serial.print("subscribing to : ");
      Serial.print(topicCmd);
      if(client.subscribe(topicCmd)){
        Serial.println(" succeeded");
      }else{
        Serial.println(" failed");
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void work(){
    Serial.println("work");
    readSensor();
    trace();
    displayTemperature();
    sendDatas();
}

char *ftoa(char *a, double f, int precision){
 long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};
 
 char *ret = a;
 long heiltal = (long)f;
 itoa(heiltal, a, 10);
 while (*a != '\0') a++;
 *a++ = '.';
 long desimal = abs((long)((f - heiltal) * p[precision]));
 itoa(desimal, a, 10);
 return ret;
}

void sendDatas(){
  sendTemperature(temperature);
  sendPressure(pressure);
}

void sendTemperature(float aTemperature){
//  int intPart = (int)aTemperature;
//  //int decPart; = (int)((aTemperature-intPart)*100);
//  unsigned int decPart;
//  if(intPart < 0){
//    
//  }
  ftoa(msg, aTemperature, 2);
  //snprintf (msg, sizeof(msg), "%s.%s", intPart, decPart);
  snprintf (topic, sizeof(topic), "%s/%s", topicStatus, "temperature");
  sendmsgToTopic();
}

void sendPressure(int aPresure){
  snprintf (msg, sizeof(msg), "%d", aPresure);
  snprintf (topic, sizeof(topic), "%s/%s", topicStatus, "pressure");
  sendmsgToTopic();
}
void sendmsgToTopic(){
  Serial.print("Publish on: ");
  Serial.print(topic);
  Serial.print(" message: ");
  Serial.println(msg);
  client.publish(topic, msg);
}

void readSensor(){
    //Serial.println("Read bpm180");
    // READ DATA
    float myTemperature = bmp.readTemperature();
    if(myTemperature<150){
      // assumed correct read
      temperature = myTemperature;
      pressure = bmp.readPressure();
      altitude = bmp.readAltitude();
    }
}

void trace(){
    // DISPLAY DATA
    if(loopCnt++ %20 == 0){
      printHeader();
    }
    printData(temperature, pressure, altitude);  
}

void printData(float aTemperature, int aPressure, float aAltitude){
  Serial.print(aTemperature);
  Serial.print("\t");
  Serial.print(aPressure);
  Serial.print("\t");
  Serial.print(aAltitude);
  Serial.println("");
}

void printHeader(){
  Serial.println("Temperature °C\tPressure\tAltitude");  
}


/**
 * All segments on the display to on
 */
void displayAllOn(){
  // All segments on
  displayAll(0xff);
}

/**
 * All segments on the display to off
 */
void displayAllOff(){
  // All segments off
  displayAll(0);
}

/**
 * Display same pattern on all segments
 */
void displayAll(uint8_t aValue){
  // All segments on the same value
  Serial.print("all to ");
  Serial.println(aValue);
  uint8_t data[] = { aValue, aValue, aValue, aValue };
  display.setSegments(data);
}

/**
 * Display on the embedded hardware
 */
void displayTemperature(){
    //Serial.print("Display temperature : ");  
    //Serial.print(temperature, 1);
    //Serial.println("°C");
  
    display.showNumberDec(temperature * 10, false, 4, 0);
}


