#include "SparkFunMicroOLED/SparkFunMicroOLED.h"  // Include Micro OLED library
#include "Particle_BaroSensor/Particle_BaroSensor.h"
#include "math.h"
#define ADDRESS_Si7020	0x40
#define READS               2
MicroOLED oled;
void setup()
{ 
  oled=MicroOLED(MODE_SPI, D7, D6, A6);//Pinout role definition for OLED
  pinMode(D7, OUTPUT);
  oled.begin();    // Initialize the OLED
  oled.clear(ALL); // Clear the display's internal memory
  oled.display();  // Display what's in the buffer 
  delay(1000);     // Delay 1000 ms
  oled.clear(PAGE); // Clear the buffer.
}

void loop()
{ 
          oled.clear(PAGE); 
          readVars();
          delay(3000);
          pixelExample();

}

float temperatureRead(void)
{
    int storedata=0;
    for(int n=0;n<READS;n++)
        {
            Wire.begin();
            Wire.beginTransmission(ADDRESS_Si7020);   // "start" i2c
            Wire.write(0xF3);                         // read command for temperature
            Wire.endTransmission();                   // "stop" i2c
            delay(20);                                // 20msec that the device needs for data acquisition
            Wire.requestFrom(ADDRESS_Si7020, 3);      // request for a 3 bytestransmission
            unsigned char msb = Wire.read();
            unsigned char lsb = Wire.read();
            unsigned char chk = Wire.read();          // not used
            int sot=(((int) msb<<8) + (int) lsb);     // create "raw data" variable
            storedata=storedata+sot;                    // store data for average value calculation
        }
    return ((175.72*(storedata/READS))/65536)-46.85;   // calculation of the average value and transformation in °C
}

float humidityRead(void)
{
    int storedata=0;
    for(int n=0;n<READS;n++)
        {
            Wire.begin();
            Wire.beginTransmission(ADDRESS_Si7020);   // "start" i2c
            Wire.write(0xF5);                         // read command for umidity
            Wire.endTransmission();                   // "stop" i2c
            delay(20);                                // 20msec that the device needs for data acquisition
            Wire.requestFrom(ADDRESS_Si7020, 3);      // request for a 3 bytestransmission
            unsigned char msb = Wire.read();
            unsigned char lsb = Wire.read();
            unsigned char chk = Wire.read();          // not used
            int sot=(((int) msb<<8) + (int) lsb);     // create "raw data" variable
            storedata=storedata+sot;                    // store data for average value calculation
        }
    
    return ((125*(storedata/READS))/65536)-6;   // calculation of the average value and transformation in umidity (relative in %)   
}

void readVars()
{
    int RSSI_WIFI=9999;
    unsigned long tempo_unix= Time.now();
    float temp=temperatureRead();
    float umi=humidityRead();
    if(WiFi.ready()) RSSI_WIFI=WiFi.RSSI(); 
    oled.clear(PAGE);          
            oled.setCursor(0, 0);       
            oled.setFontType(0);        
            oled.print("WIFI:");         
            oled.setFontType(0);       
            oled.print(String(RSSI_WIFI)); 
            oled.setCursor(0, 16);       
            oled.setFontType(0);         
            oled.print("TEMP:");
            oled.setFontType(0);
            oled.print(String(temp,1));
            oled.setCursor(0, 32);
            oled.setFontType(0);
            oled.print("UMID:");
            oled.setFontType(0);
            oled.print(String(umi,1));
            oled.display();
            delay(2000);
}

void pixelExample()
{
  for (int i=0; i<512; i++)
  {
    oled.pixel(random(oled.getLCDWidth()), random(oled.getLCDHeight()));
    oled.display();
  }
}
