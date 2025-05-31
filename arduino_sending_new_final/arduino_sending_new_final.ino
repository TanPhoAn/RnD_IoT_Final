#include <Adafruit_Sensor.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>  
#include <SoftwareSerial.h>

#define RX_PIN 3  // Chân RX của cổng nối tiếp mềm
#define TX_PIN 6 // Chân TX của cổng nối tiếp mềm

#define DECIBLE_PIN A0

SoftwareSerial espSerial(RX_PIN, TX_PIN);

//varible
int decible;
const int sampleWindow = 50;                              // Sample window width in mS (50 mS = 20Hz)
unsigned int sample; 
String soundString;
/**/

void getDecible(){
  unsigned long startMillis= millis();                   // Start of sample window
  float peakToPeak = 0;                                  // peak-to-peak level
                                                          // collect data for 50 mS
  unsigned int signalMax = 0;                            //minimum value
  unsigned int signalMin = 1024; 
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(DECIBLE_PIN);                    //get reading from microphone
      if (sample < 1024)                                  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;                           // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;                           // save just the min levels
         }
      }
   }

 
   peakToPeak = signalMax - signalMin;                    // max - min = peak-peak amplitude
   
   Serial.print("Peak:");
   Serial.println(peakToPeak);
  int db = convert(peakToPeak, 1, 200, 25, 70);           //calibrate for deciBels
  
  Serial.print("Loudness: ");
  Serial.print(db);
  Serial.println("dB");
  decible = db; 
  

  //sending data
  soundString = String(decible);
  espSerial.println(soundString);
  delay(500);
}
float convert(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
void pinDefine(){
  
  pinMode(DECIBLE_PIN, INPUT);
 
}


void setup() {
  // put your setup code here, to run once:
              
  pinDefine();
  Serial.begin(9600);
  espSerial.begin(115200);
}


void loop() {
  getDecible();
  delay(500);
}
