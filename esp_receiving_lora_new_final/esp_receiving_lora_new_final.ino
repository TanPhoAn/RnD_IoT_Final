#include <math.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>  //mqtt protocol
#include <Adafruit_Sensor.h>
#include <Arduino_JSON.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>  
#include <SoftwareSerial.h>



#define RX_PIN D2  
#define TX_PIN D3  
//SoftwareSerial espSerial(RX_PIN, TX_PIN);


//config wifi  and mqtt server for esp8266
//  ******variable******
ESP8266WebServer server(80);
JSONVar data;
WiFiClient espClient;
PubSubClient client(espClient);  //use espClient for network communication


const char* mqtt_server = "broker.hivemq.com";  //mqtt broker

float humidity, temperature_C;
float humidity_Out, temperature_C_Out;
int UV_index, sensor_value;  
int totalBee;
float weight;
float sound;

String dataReceived;

struct settings {
  char ssid[30];
  char password[30];
} user_wifi = {};

//  ******variable******

void web_handle() {
  server.on("/", handlePortal);
  server.begin();
}

void setup_wifi() {

  delay(10);
  EEPROM.begin(sizeof(struct settings));
  EEPROM.get(0, user_wifi);
  //config wifi station mode(join exists network)
  WiFi.mode(WIFI_STA);


  WiFi.begin(user_wifi.ssid, user_wifi.password);
  /**/
  byte tries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (tries++ > 10) {
      WiFi.mode(WIFI_AP);
      WiFi.softAP("Setup Portal", "mrdiy.ca");
      break;
    }

    Serial.print(".");
  }
  web_handle();
  Serial.println("WiFi connected");
  Serial.print("IP HOST: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.gatewayIP());
  Serial.println(WiFi.subnetMask());
  Serial.println(WiFi.status());
}
/**/
void handlePortal() {
  if (server.method() == HTTP_POST) {

    strncpy(user_wifi.ssid, server.arg("ssid").c_str(), sizeof(user_wifi.ssid));
    strncpy(user_wifi.password, server.arg("password").c_str(), sizeof(user_wifi.password));
    user_wifi.ssid[server.arg("ssid").length()] = user_wifi.password[server.arg("password").length()] = '\0';
    EEPROM.put(0, user_wifi);
    EEPROM.commit();

    server.send(200, "text/html", "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Wifi Setup</title><style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1,p{text-align: center}</style> </head> <body><main class='form-signin'> <h1>Wifi Setup</h1> <br/> <p>Your settings have been saved successfully!<br />Please restart the device.</p></main></body></html>");
  } else {

    server.send(200, "text/html", "<!doctype html> <html lang='en'> <head> <meta charset='utf-8'> <meta name='viewport' content='width=device-width, initial-scale=1'> <title>Wifi Setup</title> <style> *, ::after, ::before { box-sizing: border-box; } body { margin: 0; font-family: 'Segoe UI', Roboto, 'Helvetica Neue', Arial, 'Noto Sans', 'Liberation Sans'; font-size: 1rem; font-weight: 400; line-height: 1.5; color: #212529; background-color: #f5f5f5; } .form-control { display: block; width: 100%; height: calc(1.5em + .75rem + 2px); border: 1px solid #ced4da; } button { cursor: pointer; border: 1px solid transparent; color: #fff; background-color: #007bff; border-color: #007bff; padding: .5rem 1rem; font-size: 1.25rem; line-height: 1.5; border-radius: .3rem; width: 100% } .form-signin { width: 100%; max-width: 400px; padding: 15px; margin: auto; } h1 { text-align: center } </style> </head> <body> <main class='form-signin'> <form action='/' method='post'> <h1 class=''>Wifi Setup For BeeHive Equipment</h1><br /> <div class='form-floating'><label>Enter the Wifi's username:</label><input type='text' class='form-control' name='ssid'> </div> <div class='form-floating'><br /><label>Enter the Wifi's password</label><input type='password' class='form-control' name='password'></div><br /><br /><button type='submit'>Save Change</button> </form> </main> </body> </html>");
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      //if esp can connect to server, then esp will subcribe the topic
      client.subscribe("device/temp");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
//receive message sent from device to mqtt
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // if message = 1 then react on esp
  if ((char)payload[0] == '1') {
    Serial.print("esp8266 receive message 1");
  }
}

void dtReceive(){
  if (Serial.available()) {
    dataReceived = Serial.readStringUntil('\n');  // Read from Arduino
    Serial.println(dataReceived);
  }
}

void parseData(String input) {
  int start = 0;

  while (start < input.length()) {
    int colon = input.indexOf(':', start);
    int comma = input.indexOf(',', start);
    if (comma == -1) comma = input.length();

    if (colon != -1 && colon < comma) {
      String key = input.substring(start, colon);
      String value = input.substring(colon + 1, comma);

      if (key == "tei") temperature_C = value.toFloat();
      else if (key == "hui") humidity = value.toFloat();
      else if (key == "teo") temperature_C_Out = value.toFloat();
      else if (key == "huo") humidity_Out = value.toFloat();
      else if (key == "uvv") sensor_value = value.toInt();
      else if (key == "uvi") UV_index = value.toInt();
      else if (key == "w")   weight   = value.toFloat();
      else if (key == "B")   totalBee   = value.toInt();
      else if (key == "s")   sound   = value.toFloat();
    }

    start = comma + 1;
  }
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
  client.publish("beehive_hive_1", jsonString.c_str(), true);
  Serial.println(jsonString.c_str());
}

void dataPublishing1(){
  //data format
    data["receive"] = dataReceived;
    //convert object to json string
    String jsonString = JSON.stringify(data);

  //publish sensor data to server via topic
  client.publish("hive_bee_1", jsonString.c_str(), true);
  Serial.println(jsonString.c_str());
}




void setup() {
  Serial.begin(115200);
  //espSerial.begin(115200);

  setup_wifi();
  

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}


void loop() {
  
  while (WiFi.status() != WL_CONNECTED) { // wait until wifi is connected
        Serial.print(".");
        delay(500);
        server.handleClient(); // handle http request if needed
    }
  delay(1000);
  if (!client.connected()) {
    reconnect();
   }
   client.loop();
  //Wait a few seconds between measurements.

     dtReceive();
     parseData(dataReceived);
     dataPublishing();
    delay(10000);
    
}
