#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <dht.h>

// Sensor definitions
#define DHTPIN  5

// 7 segments settings
#define LATCH 12
#define CLK   14
#define DATA  16

//This is the hex value of each number stored in an array by index num
const byte digitOne[10]= {0xDE, 0x06, 0xEC, 0xAE, 0x36, 0xBA, 0xFA, 0x0E, 0xFE, 0xBE};
//const byte digitOne[10]= {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00};

// Wifi settings
const char* ssid     = "MaisonSMT";
const char* password = "m3f13t01";

// http web server
ESP8266WebServer server(80);
 
// Initialize DHT sensor 
dht DHT;

String webString="";     // String to display
// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2000;              // min time between sensor read in ms


void setup(){

  // You can open the Arduino IDE Serial Monitor window to see what the code is doing
  Serial.begin(115200);
  Serial.println("setup");

  //
  // 7 segment ports
  //
  
  pinMode(LATCH, OUTPUT);
  pinMode(CLK,   OUTPUT);
  pinMode(DATA,  OUTPUT);

  //
  // WIFI
  //
  
  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.print("\n\r \n\rWorking to connect");
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("DHT Weather Reading Server");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //
  // HTTP server
  //
  
  server.on("/", handle_root);  
  server.on("/temp", handle_temp);
  server.on("/humidity", handle_humidity);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop(){
  server.handleClient();
  // doDigitCheck();
}

void handle_root() {
  server.send(200, "text/html", "<html><body><h1>Hello from the weather esp8266</h1><p>read from</p> <a href='/temp'>/temp</a> or <a href='/humidity'>/humidity</aS></body></html>");
//  delay(100);
}

void handle_temp() {
  // if you add this subdirectory to your webserver call, you get text below :)
    getTemperature();       // read sensor
    webString="Temperature: "+String((int)DHT.temperature)+" °C";   // Arduino has a hard time with float to string
    server.send(200, "text/plain", webString);            // send to someones browser when asked
}

void handle_humidity() {
// if you add this subdirectory to your webserver call, you get text below :)
    getTemperature();           // read sensor
    webString="Humidity: "+String((int)DHT.humidity)+"%";
    server.send(200, "text/plain", webString);               // send to someones browser when asked
}

void doDigitCheck(){
  for(int i=0; i<10; i++){
    Serial.print("digit : ");
    Serial.println(i);
      to7SegmentDigit(i);
      delay(500);
  }
}

void to7SegmentDigit(int digit1){
  Serial.print("Display 7 segments : ");  
  Serial.println(digit1);

  digitalWrite(LATCH, LOW);
  digitalWrite(CLK, LOW);
  shiftOut(DATA, CLK, MSBFIRST, digitOne[digit1 % 10]); // digitOne
  digitalWrite(LATCH, HIGH);
}

void displayTemperature(){
    Serial.print("Display temperature on 7 segments : ");  
    Serial.print(DHT.temperature, 1);
    Serial.println("°C");

    to7SegmentDigit(DHT.temperature/10);
}

void getTemperature() {
  // Wait at least 2 seconds seconds between measurements.
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor 
    previousMillis = currentMillis;   

    Serial.println("Reading DHT22 sensor");
    int chk = DHT.read22(DHTPIN);

    switch (chk)
    {
    case DHTLIB_OK:
        Serial.print("OK,\t");
        break;
    case DHTLIB_ERROR_CHECKSUM:
        Serial.print("Checksum error,\t");
        break;
    case DHTLIB_ERROR_TIMEOUT:
        Serial.print("Time out error,\t");
        break;
    default:
        Serial.print("Unknown error,\t");
        break;
    }

    // DISPLAY DATA
    Serial.print(DHT.humidity, 1);
    Serial.print("%,\t");
    Serial.print(DHT.temperature, 1);
    Serial.println("°C");

    displayTemperature();
  }
}
