#include <SPI.h>
#include "Adafruit_MAX31855.h"
#define MAXDO   3
#define MAXCS   4
#define MAXCLK  5
#define fanPin 10
#define heatPin 11
#define caseFan 12
#include <WiFiNINA.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "arduino_secrets.h" 
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);
char ssid[] = SECRET_SSID;      
char pass[] = SECRET_PASS;   
int keyIndex = 0;  
int status = WL_IDLE_STATUS;
int cycles = 20;
WiFiServer server(80);
void setup() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    Serial.println("fuck");
    for(;;);
    } // Don't proceed, loop forever
      display.display();
      display.clearDisplay();
      display.setTextSize(2); 
      display.setTextColor(SSD1306_WHITE);
       display.cp437(true);         
  display.write("Welcome toElam's PCRMachine!  ");
  display.println("booting...");
  display.display();
  pinMode(fanPin, OUTPUT);
  pinMode(heatPin, OUTPUT);
  pinMode(caseFan, OUTPUT);
  delay(500);
 Serial.begin(9600);      // initialize serial communication
  pinMode(9, OUTPUT);      // set the LED pin mode

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();                           // start the web server on port 80                 // you're connected now, so print out the status
 IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println("");
      display.clearDisplay();
      display.setTextSize(2);
  display.setCursor(0,0);
  display.println("go to ");
        display.setTextSize(1);
  display.println(String(ip[0]) + "." +  String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]));
        display.setTextSize(1);
  display.println("to get started!");
  display.display();
}

boolean holdConstantTemp(long duration, double idealTemp){
  Serial.println("");
  Serial.print("holding "); 
  Serial.print(idealTemp);
  Serial.print(" for   ");  
  Serial.print(duration);
    double currentTemp;
   unsigned long startTime = millis();
  long timeElapsed = millis() - startTime;
    currentTemp = thermocouple.readCelsius();
  while (timeElapsed < duration){
      double tmp = thermocouple.readCelsius();
    if (!isnan(tmp)){
          currentTemp = thermocouple.readCelsius();
    }
  if (currentTemp < idealTemp){
    digitalWrite(heatPin, HIGH);
    delay(90);
    digitalWrite(heatPin, LOW);
  }
  else if (currentTemp > (idealTemp+0.5)){
    digitalWrite(fanPin,HIGH);
    delay(90);
    digitalWrite(fanPin,LOW);
  }
  delay(210);
  timeElapsed = millis() - startTime;
}
  return true;
}
boolean reachTemp(double desiredTemp){ // finish with cooling too
  Serial.println("reaching");
  Serial.print(desiredTemp);
  double currentTemp = thermocouple.readCelsius();
  while(desiredTemp > currentTemp){
         double tmp = thermocouple.readCelsius();
    if (!isnan(tmp)){
          currentTemp = thermocouple.readCelsius();
    }
     digitalWrite(heatPin, HIGH);
     Serial.println("heating on");
     Serial.print("temp=");
     Serial.println(currentTemp);
          Serial.println("");
     delay(1000);
  }   
  while(desiredTemp < currentTemp){
    double tmp = thermocouple.readCelsius();
    if (!isnan(tmp)){
          currentTemp = thermocouple.readCelsius();
    }

    digitalWrite(fanPin, HIGH);
     Serial.println("fan on");
     Serial.print("temp=");
     Serial.println(currentTemp);
          Serial.println("");
  }
  digitalWrite(heatPin,LOW);
  digitalWrite(fanPin,LOW);
  return true;
}
void loop() {
  WiFiClient client = server.available();   
  if (client) {                             
    Serial.println("new client");           
    String currentLine = "";                
    while (client.connected()) {            
      if (client.available()) {            
        char c = client.read();            
        Serial.write(c);                    
        if (c == '\n') {                  
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Welcome to Elam's PCR machine!<br>");
            client.print("Click <a href=\"/start\">here</a>to start with 30 cycles!<br>");
            client.print("Click <a href=\"/warm\">here</a>to warm up the machine and hold at 94C!<br>");
            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if client request was warm or start

        if (currentLine.endsWith("GET /warm")){
                  digitalWrite(caseFan, HIGH);
          reachTemp(94.0);
          while(!currentLine.endsWith("GET /start")){
              holdConstantTemp(30000, 94.0);  
          }
        }
        if (currentLine.endsWith("GET /start")) {
                  digitalWrite(caseFan, HIGH);
          holdConstantTemp(120000,94.0); // inital denaturing
          for (int i = 0; i <= cycles; i++){
            Serial.print("cycle " + String(i) + " of " + String(cycles));
            client.print("cycle " + String(i) + " of " + String(cycles));
            holdConstantTemp(15000, 94.0); // denaturing
            holdConstantTemp(20000, 55.0);
            holdConstantTemp(60000,72.0);// 1 min per kb, setting to 1 min for now
          }
          holdConstantTemp(300000,72.0);
          while(true){
            holdConstantTemp(30000, 4.0);
          }
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }  
}
