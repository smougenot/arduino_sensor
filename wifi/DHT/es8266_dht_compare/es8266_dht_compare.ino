/* DHTServer - ESP8266 Webserver with a DHT sensor as an input
 
   Based on ESP8266Webserver, DHTexample, and BlinkWithoutDelay (thank you)
 
   Version 1.0  5/3/2014  Version 1.0   Mike Barela for Adafruit Industries
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

#define DHTTYPE1 DHT11
#define DHTPIN1  5
#define DHTTYPE2 DHT22
#define DHTPIN2  2
 
const char* ssid     = "MaisonSMT";
const char* password = "m3f13t01";
 
ESP8266WebServer server(80);
 
// Initialize DHT sensor 
// NOTE: For working with a faster than ATmega328p 16 MHz Arduino chip, like an ESP8266,
// you need to increase the threshold for cycle counts considered a 1 or 0.
// You can do this by passing a 3rd parameter for this threshold.  It's a bit
// of fiddling to find the right value, but in general the faster the CPU the
// higher the value.  The default for a 16mhz AVR is a value of 6.  For an
// Arduino Due that runs at 84mhz a value of 30 works.
// This is for the ESP8266 processor on ESP-01 
DHT dht1(DHTPIN1, DHTTYPE1, 11); // 11 works fine for ESP8266
DHT dht2(DHTPIN2, DHTTYPE2, 11); // 11 works fine for ESP8266
 
float humidity1, temp1, humidity2, temp2;  // Values read from sensor
String webString="";     // String to display
// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2000;              // interval at which to read sensor
 
void handle_root() {
  server.send(200, "text/html", "<html><body><h1>Hello from the weather esp8266</h1><p>read from</p> <a href='/temp'>/temp</a> or <a href='/humidity'>/humidity</aS></body></html>");
  delay(100);
}
 
void setup(void)
{
  // You can open the Arduino IDE Serial Monitor window to see what the code is doing
  Serial.begin(115200);  // Serial connection from ESP-01 via 3.3v console cable
  dht1.begin();           // initialize temperature sensor
  dht2.begin();           // initialize temperature sensor
 
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
   
  server.on("/", handle_root);
  
  server.on("/temp", [](){  // if you add this subdirectory to your webserver call, you get text below :)
    gettemperature();       // read sensor
    webString="Temperature: "
      +String((int)temp1)+" F"
      + " ; "
      +String((int)temp2)+" F"
      ;   // Arduino has a hard time with float to string
    server.send(200, "text/plain", webString);            // send to someones browser when asked
  });
 
  server.on("/humidity", [](){  // if you add this subdirectory to your webserver call, you get text below :)
    gettemperature();           // read sensor
    webString="Humidity: "
      +String((int)humidity1)+"%"
      + " ; "
      +String((int)humidity2)+"%"
      ;
    server.send(200, "text/plain", webString);               // send to someones browser when asked
  });
  
  server.begin();
  Serial.println("HTTP server started");
}
 
void loop(void)
{
  server.handleClient();
} 
 
void gettemperature() {
  // Wait at least 2 seconds seconds between measurements.
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor 
    previousMillis = currentMillis;   

    Serial.println("Reading sensor");
 
    // Reading temperature for humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
    humidity1 = dht1.readHumidity();       // Read humidity (percent)
    temp1 = dht1.readTemperature(false);   // Read temperature as Celcius
    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity1) || isnan(temp1)) {
      Serial.println("Failed to read from DHT sensor!");
      humidity1 = temp1 = -1;
    }

    humidity2 = dht2.readHumidity();       // Read humidity (percent)
    temp2 = dht2.readTemperature(false);   // Read temperature as Celcius
    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity2) || isnan(temp2)) {
      Serial.println("Failed to read from DHT sensor!");
      humidity2 = temp2 = -1;
    }

  }
}
