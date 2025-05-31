#include <math.h>
#include "DHT.h"
#include <Adafruit_Sensor.h>
#include <Arduino_JSON.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>  
#include <SoftwareSerial.h>

#define DHTPIN 4 //D2
#define DHTPIN1 14 //D5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
DHT dht1(DHTPIN1, DHTTYPE);
#define OBSTACLE_PIN D1
#define UV_PIN A0
#define SCALE_DATA_PIN D4
#define SCALE_SCK_PIN D3

#define RX_PIN 2 //D4 ko xài
#define TX_PIN 13 //D7
SoftwareSerial espSerial(RX_PIN, TX_PIN);


//config wifi  and mqtt server for esp8266
//  ******variable******

JSONVar data;

float humidity, temperature_C, temperature_F;
float humidity_Out, temperature_C_Out, temperature_F_Out;
double UV_index, sensor_value;  
double totalBee;
float weight;
unsigned long x, y = 0;
unsigned long dataArray[10];
int j = 0;
String soundData;
float sound;


//  ******variable******


/**/


//receive message sent from device to mqtt

void getTempHumid() {
  temperature_C = dht.readTemperature();

  temperature_F = dht.readTemperature(true);

  humidity = dht.readHumidity();



  // Check for error reading
  if (isnan(humidity) || isnan(temperature_C) || isnan(temperature_F)) {
    //Serial.println(" DHT reading failed ");
    return;
  }


  //Serial.print("Humidity: ");
  //Serial.print(humidity);
  //Serial.println("%");

  //Serial.print("Temperature:");
  //Serial.print(temperature_C);
  //Serial.print(("°C ------- "));

  //Serial.print(temperature_F);
  //Serial.println("°F");
}

void getTempHumidOut() {
  temperature_C_Out = dht1.readTemperature();

  temperature_F_Out = dht1.readTemperature(true);

  humidity_Out = dht1.readHumidity();



  // Check for error reading
  if (isnan(humidity_Out) || isnan(temperature_C_Out) || isnan(temperature_F_Out)) {
    //Serial.println(" DHT reading failed ");
    return;
  }


  // Serial.print("Humidity: ");
  // Serial.print(humidity_Out);
  // Serial.println("%");

  // Serial.print("Temperature:");
  // Serial.print(temperature_C_Out);
  // Serial.print(("°C ------- "));

  // Serial.print(temperature_F_Out);
  // Serial.println("°F");
}

void getUV(){
  sensor_value = analogRead(UV_PIN); 
  float volts = sensor_value * 5.0 / 1024.0;
  UV_index = volts * 10;
 // Serial.print ("Raw ADC data: ");
  //Serial.print (sensor_value);
  //Serial.print ("  UV Index: ");
  //Serial.println (UV_index);
}

void getIR(){
  //read if there is obstacle
  //Serial.print("obs \t");
  int obsDetect = digitalRead(OBSTACLE_PIN);
  if(obsDetect == 0){
    totalBee +=1;
    
  }
  //Serial.println(totalBee);
}

void getWeight(){

    unsigned long timeout = millis() + 1000; // 1-second timeout
    while (digitalRead(SCALE_DATA_PIN) != LOW) {
      if (millis() > timeout) {
        // Serial.println("Error: HX711 not responding (DATA_PIN stuck HIGH)");
        return; // Exit loop to prevent WDT reset
      }
      yield(); // Allow background tasks to run
    }
  
    for (int j = 0; j < 10; j++)
    {
      digitalWrite(SCALE_SCK_PIN, LOW);//SCK is made LL
      while (digitalRead(SCALE_DATA_PIN) != LOW) //wait until Data Line goes LOW
        ;
      {
        for (int i = 0; i < 24; i++)  //read 24-bit data from HX711
        {
          clk();      //generate CLK pulse to get MSB-it at SCALE_DATA_PIN-pin
          bitWrite(x, 0, digitalRead(SCALE_DATA_PIN));
          x = x << 1;
        }
        clk();
        y = x;
        x = 0;
        delay(400);
      }
      dataArray[j] = y;
    }
  
    //Serial.println("===averaging process=========");
    unsigned long sum = 0;
  
    for (j = 0; j < 10; j++)
    {
      sum += dataArray[j];
    }
    //Serial.print("Average Count = ");
    sum = sum / 10;
    //Serial.println(sum, DEC);
    //float tare = 0.790 ;
    float W = (float)(5.477*sum)/1000000 - 2.125;
    
    //Serial.println(W, 4);
    weight = W;
}

void clk(){
  digitalWrite(SCALE_SCK_PIN, HIGH);
  digitalWrite(SCALE_SCK_PIN, LOW);
}
void soundReceive(){
  while (!espSerial.available()) {
    delay(1000);
  }
  if(espSerial.available()){
    soundData = espSerial.readStringUntil('\n');  // Read from Arduino
    sound = atof(soundData.c_str());
    //Serial.println(sound);
  }
    
  
}

void dataTransfer(){
  //data format
    double round_temperature_C = round(temperature_C * 1000) / 1000.0;
    data["tei"] = round_temperature_C;
    double round_humidity = round(humidity * 1000) / 1000.0;
    data["hui"] = round_humidity;
    double round_temperature_C_Out = round(temperature_C_Out * 1000) / 1000.0;
    data["teo"] = round_temperature_C_Out;
    double round_humidity_Out = round(humidity_Out * 1000) / 1000.0;
    data["huo"] = round_humidity_Out;
    data["pl"] = "DN";
    data["uvv"] = sensor_value;
    data["uvi"] = (int)UV_index;
    double round_weight = round(weight * 1000) / 1000.0;
    data["w"] = round_weight;
    data["B"] = (int)totalBee; 
    data["s"] = sound;
    //convert object to json string
    String jsonString = JSON.stringify(data);
    String total = jsonString + "\n";
  //publish sensor data to server via topic

    Serial.println(total);
    
    
  
}

void dataTransfer2() {
  // Làm tròn dữ liệu
  double round_temperature_C = round(temperature_C * 1000) / 1000.0;
  double round_humidity = round(humidity * 1000) / 1000.0;
  double round_temperature_C_Out = round(temperature_C_Out * 1000) / 1000.0;
  double round_humidity_Out = round(humidity_Out * 1000) / 1000.0;
  double round_weight = round(weight * 1000) / 1000.0;

  // Tạo chuỗi
  String total = "tei:" + String(round_temperature_C);
 // Serial.println(total);
  
  total += ",hui:" + String(round_humidity);
  total += ",teo:" + String(round_temperature_C_Out);
  total += ",huo:" + String(round_humidity_Out);
  total += ",pl:DN";
  total += ",uvv:" + String(sensor_value);
  total += ",uvi:" + String((int)UV_index);
  total += ",w:" + String(round_weight, 3);
  total += ",B:" + String((int)totalBee);
  total += ",s:" + String(sound);
  //Stotal += "\n";
  // In ra Serial
 espSerial.println(total);
 Serial.println(total);
}


void dataPublishing(){
  //data format
    double round_temperature_C = roundf(temperature_C * 1000) / 1000.0;
    data["temperature_inside"] = round_temperature_C;
    double round_humidity = roundf(humidity * 1000) / 1000.0;
    data["humidity_inside"] = round_humidity;
    double round_temperature_C_Out = roundf(temperature_C_Out * 1000) / 1000.0;
    data["temperature_outside"] = round_temperature_C_Out;
    double round_humidity_Out = roundf(humidity_Out * 1000) / 1000.0;
    data["humidity_outside"] = round_humidity_Out;
    data["place"] = "Dong Nai";
    data["uv_value"] = sensor_value;
    data["uv_index"] = (int)UV_index;
    data["weight"] = (double)weight;
    data["totalBee"] = totalBee; 
    data["sound"] = sound;
    //convert object to json string
    String jsonString = JSON.stringify(data);

  //publish sensor data to server via topic
  
  Serial.println(jsonString.c_str());
}


void getSensors(){
  //soundReceive();
  getUV();
  getTempHumid();
  getTempHumidOut();
  getIR();
  getWeight();
}

void pinDefine(){

  pinMode(OBSTACLE_PIN, INPUT);
  pinMode(UV_PIN, INPUT);
  pinMode(SCALE_DATA_PIN, INPUT); //data line  //Yellow cable
  pinMode(SCALE_SCK_PIN, OUTPUT);
  
}

void setup() {
  Serial.begin(115200);
  espSerial.begin(115200);
  //Serial.setTimeout(3000);
  pinDefine();

  
  dht.begin();
  dht1.begin();
  delay(2000);

  getSensors();
  dataTransfer2();
  delay(2000);
  ESP.deepSleep(12e6);
}


void loop() {
   //delay(3000);
  
  
  
  //ESP.deepSleep(12e6);
  // while (WiFi.status() != WL_CONNECTED) { // wait until wifi is connected
  //       Serial.print(".");
  //       delay(500);
  //       server.handleClient(); // handle http request if needed
  //   }

  // if (!client.connected()) {
  //   reconnect();
  //  }
  //  client.loop();
  //Wait a few seconds between measurements.
}
