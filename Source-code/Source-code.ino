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

//VARs and PINs
#define RAIN A1
#define LED A0
#define EthernetCS 10

#define MAX_ALLOWED_TEMPERATURE 60
#define MIN_ALLOWED_PRESSURE 0
#define MAX_BMP_RESET_ATTEMPTS 5
float Temperature, Pressure, ApproxAltitude, RainLevel;
unsigned daysPassed = 0;

// BMP setup
Adafruit_BMP280 bmp;
#define BMPaddr 0x76
bool BMPstatus, BMPlastStatus;

/*

THE DHT11 DOES NO LOGER WORK ON THE IWS

#define DHTPIN A2     
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
*/

//LAN
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 200);
EthernetServer server(80);

void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  pinMode(A2, OUTPUT);
  digitalWrite(A2, LOW); // prevent short circuits
  pinMode(RAIN, INPUT);
  Ethernet.init(EthernetCS);

  Serial.begin(9600);
  
  //STATUP

  Ethernet.begin(mac, ip);
  BMPstatus = initBMP();
  BMPlastStatus = BMPstatus;
  server.begin();

  checkHW(BMPstatus);
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

          takeMeasurements();

          if(Temperature >= MAX_ALLOWED_TEMPERATURE || Pressure <= MIN_ALLOWED_PRESSURE || !BMPstatus){
            BMPlastStatus = false;
            int i = 0;
            while (Temperature >= MAX_ALLOWED_TEMPERATURE || Pressure <= MIN_ALLOWED_PRESSURE || !BMPstatus){
              if(i == MAX_BMP_RESET_ATTEMPTS){
                holdAndWait();
              }
              BMPstatus = initBMP();
              takeMeasurements();
              i += 1;
            }
            
          }


          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json");
          client.println("Connection: close");
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
          if(BMPlastStatus == false){
            client.println("\"IWSmessage\":\"The BMP resetted, hoping data will now be right.\"");
          } else {
            client.println("\"IWSmessage\":\"All clear.\"");
          }
          client.println("}");
          
          BMPlastStatus = true;
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

bool initBMP(){
  bool output = bmp.begin(BMPaddr);

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  return output;
}

void checkHW(bool BMPls){
  bool output = true;
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      output = false;
    }
  if (Ethernet.linkStatus() == LinkOFF) {
    output = false;
  }

  output = output && BMPls;

  if(output == 0 ){
    holdAndWait();
  }
}

void holdAndWait(){ //When this function gets called, it stops every process and puts the IWS in HOLD state
  digitalWrite(LED, HIGH);
  while (true){
    delay(3000000);
  }
}

void takeMeasurements(){
  Temperature = bmp.readTemperature();
  Pressure = bmp.readPressure();
  ApproxAltitude = bmp.readAltitude(1013.25);
  RainLevel = 1023 - analogRead(RAIN);
}