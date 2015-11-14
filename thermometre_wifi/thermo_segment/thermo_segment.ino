/**
 * Little project to retreave datas and push that on the web
 * TODO :
 * 1 buffer the reading => copy data in local vars if read succèss to prevent absurd values to be sent
 * 2 make adafruit storage work
 * 3 let it work standalone for proof
 * 4 deploy own storage (maybe mqtt + casandra + dashboard on Raspberry Pi)
 */
#include <Arduino.h>
#include <TM1637Display.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <dht.h>
#include "Adafruit_IO_Client.h"
#include <Ticker.h>

// Configure Adafruit IO access.
#define AIO_KEY    "5ae061cb1ec4eec40969980fc3faa5969ae0cc27"
#define AIO_FEED   "temperature-esp8266"

// Sensor definitions
#define DHTPIN  5

// display
#define CLK   12
#define DATA  14

// Flags
#define READ_ON   1
#define READ_OFF  0

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
double humidity = -1;
double temperature = -1;


// Sensor reading timer
Ticker flipper;

// Display
TM1637Display display(CLK, DATA);

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

int doRead = READ_OFF;

void setup(){

  // You can open the Arduino IDE Serial Monitor window to see what the code is doing
  Serial.begin(115200);
  Serial.println("setup");

  //
  // Display
  //
  display.setBrightness(0x0f);
  // All segments on
  Serial.println("all");
  uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
  display.setSegments(data);

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
  // Sensor reading
  //
  
  // flip the pin every 2.5s
  flipper.attach(10, readOnNextLoop);

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

  // première mesure
  Serial.println("Lancement de la première mesure");  
  getTemperature();
  uint8_t data_clear[] = { 0x0, 0x0, 0x0, 0x0 };
  display.setSegments(data_clear);
  displayTemperature();

  doRead = READ_OFF;
}

void loop(){
  server.handleClient();
  // doDigitCheck();
  if (doRead == READ_ON){
    doRead = READ_OFF;
    doReadSensor();
  }
}

//
// Web server content generations
//

/**
 * Build the content for
 * http://<sensor>/
 */
void handle_root() {
  server.send(200, "text/html", "<html><body><h1>Hello from the weather esp8266</h1><p>read from</p> <a href='/temp'>/temp</a> or <a href='/humidity'>/humidity</aS></body></html>");
//  delay(100);
}

/**
 * Build the content for
 * http://<sensor>/temp
 */
void handle_temp() {
  // if you add this subdirectory to your webserver call, you get text below :)
    //getTemperature();       // read sensor
    webString="Temperature: "+String(temperature)+" °C";   // Arduino has a hard time with float to string
    server.send(200, "text/plain", webString);            // send to someones browser when asked
}

/**
 * Build the content for
 * http://<sensor>/humidity
 */
void handle_humidity() {
// if you add this subdirectory to your webserver call, you get text below :)
    //getTemperature();           // read sensor
    webString="Humidity: "+String((int)humidity)+"%";
    server.send(200, "text/plain", webString);               // send to someones browser when asked
}

void readOnNextLoop(){
  Serial.println(F("readOnNextLoop")); 
  doRead = READ_ON;
}

/**
 * Callback for sensor reading
 */
void doReadSensor(){
  Serial.println(F("doReadSensor")); 
  getTemperature();
  
  //
  // transmit datas
  //

  // to the embedded display
  displayTemperature();

  // to the network
  //sendTemperature2Adafruit();
}

/**
 * Trying to send datas to adafruit cloud 
 */
void sendTemperature2Adafruit(){
  Serial.print(F("Adafruit send feed: ")); 
  Serial.println(temperature, 1);
  // To write a value just call the feed's send function and pass it the value.
  // Send will create the feed on Adafruit IO if it doesn't already exist and
  // then add the value to it.  Send returns a boolean that's true if it works
  // and false if it fails for some reason.
  if (testFeed.send(temperature)) {
    Serial.print(F("Wrote value to feed: ")); 
    Serial.println(temperature, 1);
  }
  else {
    Serial.println(F("Error writing value to feed!"));
  }

}

/**
 * Display on the embedded hardware
 */
void displayTemperature(){
    Serial.print("Display temperature on 7 segments : ");  
    Serial.print(temperature, 1);
    Serial.println("°C");
  
    display.showNumberDec(temperature * 10, false, 4, 0);
}

/**
 * Gateway to the sensor
 * Takes care of the timing (do not ask the sensor too often ... it has limits)
 */
void getTemperature() {
  // Wait at least the defined interval (at least 2 seconds) between measurements.
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor 
    previousMillis = currentMillis;   

    readTemperature();

  }
}

/**
 * Technicalities for reading the sensor
 */
void readTemperature(){
    Serial.println("Reading DHT22 sensor");

    //
    // Read the sensor
    //
    
    int chk = DHT.read22(DHTPIN);

    switch (chk)
    {
    case DHTLIB_OK:
        Serial.print("OK,\t");
        temperature = DHT.temperature;
        humidity = DHT.humidity;
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
}

