#include "DHT.h"
#include <TM1637Display.h>

// Sensor definitions
#define DHTPIN  5

// display
#define CLK   12
#define DATA  14

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dhtReader;
 
// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2000;              // min time between sensor read in ms : 2 seconds mini
float humidity = -1;
float temperature = -1;

// Display
TM1637Display display(CLK, DATA);

// Stat
struct
{
   uint32_t total;
   uint32_t ok;
   uint32_t ko;
   uint32_t crc_error;
   uint32_t time_out;
   uint32_t connect;
   uint32_t ack_l;
   uint32_t ack_h;
   uint32_t unknown;
} stat = { 0,0,0,0,0,0,0,0,0};

void setup() {

  // You can open the Arduino IDE Serial Monitor window to see what the code is doing
  Serial.begin(115200);
  Serial.println("setup");

  //
  // Sensor
  //
  dhtReader.setup(DHTPIN); 

  //
  // Display
  //
  display.setBrightness(0x0f);
  // All segments on
  displayAllOn();
  
  // première mesure
  Serial.println("Lancement de la première mesure");  
  displayAllOff();
  doReadSensor();
}

void loop() {
  // Wait at least the defined interval (at least 2 seconds) between measurements.
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= 2500) {
    // read sensor
    doReadSensor();
  }
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
 * Callback for sensor reading
 */
void doReadSensor(){
  Serial.println(F("doReadSensor")); 

  // Read sensor datas
  getTemperature();
  
  //
  // transmit datas
  //

  // to the embedded display
  displayTemperature();

}

/**
 * Display on the embedded hardware
 */
void displayTemperature(){
    Serial.print("Display temperature : ");  
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
  trace();

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dhtReader.getHumidity();
  // Read temperature as Celsius (the default)
  float t = dhtReader.getTemperature();

  // Check if any reads failed and exit early (to try again).
  stat.total++;
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    stat.ko++;
  }else {
    // Update stored data
    temperature = t;
    humidity = h;
    stat.ok++;
  }
  
  // DISPLAY DATA
  Serial.print(h, 1);
  Serial.print("%,\t");
  Serial.print(t, 1);
  Serial.println("°C");
  trace();
}

void trace(){
  Serial.print("  temperature\t");
  Serial.print(temperature, 1);
  Serial.print(",\thumidity\t");
  Serial.print(humidity, 1);
  Serial.println("");
  Serial.print("  total\t");
  Serial.print(stat.total);
  Serial.print("\tok\t");
  Serial.print(stat.ok);
  Serial.print("\tko\t");
  Serial.print(stat.ko);
  Serial.println("");   
}

