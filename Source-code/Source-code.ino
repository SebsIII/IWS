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
#include "DHT.h"
#include <Adafruit_BMP280.h>

#define DHTPIN A2
#define DHTTYPE DHT11
#define RAIN A1
#define LED A0

Adafruit_BMP280 bmp;
DHT dht(DHTPIN, DHTTYPE);

// Vars nomenclature: SENSORvalueValue2

float DHTtemp, DHThum, BMPtemp, BMPpress, BMPapproxAltitude, RAINvalue;
unsigned BMPstatus, daysPassed = 0;

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 200); 

EthernetServer server(80);

void setup() {
  //Startup pinModes and inits
  pinMode(LED, OUTPUT);
  pinMode(RAIN, INPUT);
  digitalWrite(LED, LOW);
  Ethernet.init(10);

  Serial.begin(9600);
  Ethernet.begin(mac, ip);

  BMPstatus = bmp.begin(0x76);

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  dht.begin();

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.");
    digitalWrite(LED, HIGH);
    while (true) {
      delay(1000);
    }
  }

  server.begin();
  Serial.println("Shouldn't rain today huh?");
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

          BMPtemp = bmp.readTemperature();
          BMPpress = bmp.readPressure();
          BMPapproxAltitude = bmp.readAltitude(1013.25);
          DHTtemp = dht.readTemperature();
          DHThum = dht.readHumidity();
          RAINvalue = analogRead(RAIN);

          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json");
          client.println("Connection: close");
          //client.println("Refresh: 5"); 
          client.println();
          
          client.println("{");
          client.println("\"Data\": {");
            client.print("  \"BMPtemp\":");
            client.print(BMPtemp);
            client.println(",");
            client.print("  \"BMPpress\":");
            client.print(BMPpress);
            client.println(",");
            client.print("  \"BMPapproxAltitude\":");
            client.print(BMPapproxAltitude);
            client.println(",");
            client.print("  \"DHTtemp\":");
            client.print(DHTtemp);
            client.println(",");
            client.print("  \"DHThum\":");
            client.print(DHThum);
            client.println(",");
            client.print("  \"RAINvalue\":");
            client.println(RAINvalue);
          client.println("},");
          client.print("\"Millis\":");
          client.print(millis());
          client.println(",");
          client.print("\"DaysPassed\":");
          client.print(daysPassed);
          client.println(",");
          client.println("\"IWSmessage\":\"Never kill the inner child.\"");
          client.println("}");
          
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
  delay(50);
}

int updateDaysPassed(int ms){
  ms = ms/1000;
  if(ms/86400 >= 1){
    daysPassed += 1;
  }
  return daysPassed;
}