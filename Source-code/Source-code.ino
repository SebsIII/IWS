/*
  IWS base code
  based on Arduino built-in example Web server examples

  ETH SHIELD -> STM32
  
     SCK MISO
      | |
  [ - - -] ICSP INTERFACE
  [ - - -]
      |
    MOSI
  
  MISO -> D12
  MOSI -> D11
  SCK -> D13

  By Sebs_ (https://www.github.com/SebsIII)
 */

#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
//#include "DHT.h"
#include <Adafruit_BMP280.h>

//#define DHTPIN A2     THE DHT11 DOES NO LOGER WORK ON THE IWS
//#define DHTTYPE DHT11
#define RAIN A1
#define LED A0
#define MAX_ALLOWED_TEMPERATURE 60

Adafruit_BMP280 bmp;
//DHT dht(DHTPIN, DHTTYPE);


float Temperature, Pressure, ApproxAltitude, RainLevel, ToD;
unsigned daysPassed = 0;
bool BMPstatus;

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 200); 

EthernetServer server(80);

void setup() {
  //Startup pinModes and inits
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  
  pinMode(A2, OUTPUT);
  digitalWrite(A2, LOW); // prevent short circuits
  pinMode(RAIN, INPUT);
  Ethernet.init(10);

  Serial.begin(9600);
  Ethernet.begin(mac, ip);

  BMPstatus = bmp.begin(0x76);

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  BMPstatus = true;

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.");
    digitalWrite(LED, HIGH);
    while (true) {
      delay(1000);
    }
  }

  server.begin();
  Serial.println("Shouldn't rain today huh?");
  digitalWrite(LED, LOW);
}


void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    digitalWrite(LED, HIGH);
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n' && currentLineIsBlank) {

          Temperature = bmp.readTemperature();
          Pressure = bmp.readPressure();
          ApproxAltitude = bmp.readAltitude(1013.25);
          RainLevel = analogRead(RAIN);

          if(Temperature >= MAX_ALLOWED_TEMPERATURE){
            BMPstatus = false;
            if(!ToD){
              ToD = millis();
            }
            bmp.begin(0x76);

            bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

            Temperature = bmp.readTemperature();
            Pressure = bmp.readPressure();
            ApproxAltitude = bmp.readAltitude(1013.25);
            RainLevel = analogRead(RAIN);
          }


          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json");
          client.println("Connection: close");
          //client.println("Refresh: 5"); 
          client.println();
          
          client.println("{");
          client.println("\"Data\": {");
            client.print("  \"Temperature\":");
            client.print(Temperature);
            client.println(",");
            client.print("  \"Pressure\":");
            client.print(Pressure);
            client.println(",");
            client.print("  \"ApproxAltitude\":");
            client.print(ApproxAltitude);
            client.println(",");
            client.print("  \"RainLevel\":");
            client.println(RainLevel);
          client.println("},");
          client.print("\"Millis\":");
          client.print(millis());
          client.println(",");
          client.print("\"DaysPassed\":");
          client.print(daysPassed);
          client.println(",");
          if(BMPstatus == false){
            client.print("\"IWSmessage\":\"The BMP resetted, hoping data will now be right.\"");
            client.println(",");
            client.print("\ToD\":");
            client.println(ToD);
          } else {
            client.println("\"IWSmessage\":\"All clear.\"");
          }
          client.println("}");
          
          BMPstatus = true;
          digitalWrite(LED, LOW);
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
  }
  if(millis()%5000 <= 50){
    digitalWrite(LED, HIGH);
    updateDaysPassed(millis());
    digitalWrite(LED, LOW);
  }
  delay(200);
}

int updateDaysPassed(int ms){
  ms = ms/1000;
  if(ms/86400 >= 1){
    daysPassed += 1;
  }
  return daysPassed;
}