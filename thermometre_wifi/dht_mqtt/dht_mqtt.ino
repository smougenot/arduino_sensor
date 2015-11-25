// BMP180
#include <Wire.h>
#include <Adafruit_BMP085.h>
// Display
#include <TM1637Display.h>

#define PRJ_VERSION 1

// display pins
#define CLK   14
#define DATA  16

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

// Datas
float temperature = -1;
float altitude = -1;
int pressure = -1;
short loopCnt = 0;

void setup()
{
    Serial.begin(115200);
    Serial.println("sensor to mqtt");
    Serial.print("VERSION: ");
    Serial.println(PRJ_VERSION);
    Serial.println();

    //
    // Display
    //
    display.setBrightness(0x0f);
    // All segments on
    displayAllOn();

      if (!bmp.begin()) {
        Serial.println("Could not find a valid BMP180 sensor, check wiring!");
        // loop will force reboot
        while (1) {}
      }

    // première mesure
    Serial.println("Lancement de la première mesure");  
    displayAllOff();

    
    Serial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C)\tTime (us)");
}

void loop()
{
    readSensor();
    trace();
    displayTemperature();
    delay(300);
}

void readSensor(){
    Serial.println("Read bpm180");
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
    Serial.print("Display temperature : ");  
    Serial.print(temperature, 1);
    Serial.println("°C");
  
    display.showNumberDec(temperature * 10, false, 4, 0);
}


