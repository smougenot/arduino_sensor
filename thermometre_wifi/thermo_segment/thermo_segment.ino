#define ADAFRUIT_IO_DEBUG 1
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <dht.h>
#include "Adafruit_IO_Client.h"

// Configure Adafruit IO access.
#define AIO_KEY    "5ae061cb1ec4eec40969980fc3faa5969ae0cc27"
#define AIO_FEED   "temperature-esp8266"

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
String webString="";     // String to display
 
// Initialize DHT sensor 
dht DHT;
// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2000;              // min time between sensor read in ms : 2 seconds mini

//
// adafruit IO
//

// Create an ESP8266 WiFiClient class to connect to the AIO server.
WiFiClient client;

// Create an Adafruit IO Client instance.  Notice that this needs to take a
// WiFiClient object as the first parameter, and as the second parameter a
// default Adafruit IO key to use when accessing feeds (however each feed can
// override this default key value if required, see further below).
Adafruit_IO_Client aio = Adafruit_IO_Client(client, AIO_KEY);

// Finally create instances of Adafruit_IO_Feed objects, one per feed.  Do this
// by calling the getFeed function on the Adafruit_IO_FONA object and passing
// it at least the name of the feed, and optionally a specific AIO key to use
// when accessing the feed (the default is to use the key set on the
// Adafruit_IO_Client class).
Adafruit_IO_Feed testFeed = aio.getFeed(AIO_FEED);

// Alternatively to access a feed with a specific key:
//Adafruit_IO_Feed testFeed = aio.getFeed("esptestfeed", "...esptestfeed key...");



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
  // Adafruit IO
  //
  
  // Initialize the Adafruit IO client class (not strictly necessary with the
  // client class, but good practice).
  aio.begin();

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

void sendTemperature2Adafruit(){
  Serial.print(F("Adafruit send feed: ")); 
  Serial.println(DHT.temperature, 1);
  // To write a value just call the feed's send function and pass it the value.
  // Send will create the feed on Adafruit IO if it doesn't already exist and
  // then add the value to it.  Send returns a boolean that's true if it works
  // and false if it fails for some reason.
  if (testFeed.send(DHT.temperature)) {
    Serial.print(F("Wrote value to feed: ")); 
    Serial.println(DHT.temperature, 1);
  }
  else {
    Serial.println(F("Error writing value to feed!"));
  }

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
  // Wait at least the defined interval (at least 2 seconds) between measurements.
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor 
    previousMillis = currentMillis;   

    readTemperature();

  }
}

void readTemperature(){
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
    sendTemperature2Adafruit();

}

