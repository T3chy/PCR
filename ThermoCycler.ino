#include <SPI.h>
#include "Adafruit_MAX31855.h"
#define MAXDO   3
#define MAXCS   4
#define MAXCLK  5
#define fanPin 6
#define heatPin 7
#include <SPI.h>
#include <WiFiNINA.h>
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);
char ssid[] = "hi";        // your network SSID (name)
char pass[] = "test";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;  
int status = WL_IDLE_STATUS;
WiFiServer server(80);
void setup() {
  // put your setup code here, to run once:
  pinMode(fanPin, OUTPUT);
  pinMode(heatPin, OUTPUT);
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
}
boolean holdConstantTemp(long duration, double idealTemp){
    double currentTemp;
   unsigned long startTime = millis();
  long timeElapsed = millis() - startTime;
  while (timeElapsed < duration){
  currentTemp = thermocouple.readCelsius();
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
  double currentTemp = thermocouple.readCelsius();
  while(desiredTemp > currentTemp){
     currentTemp = thermocouple.readCelsius();
     digitalWrite(heatPin, HIGH);
  }   
  while(desiredTemp < currentTemp){
    currentTemp = thermocouple.readCelsius();
    digitalWrite(fanPin, HIGH);
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
          reachTemp(94.0);
          while(!currentLine.endsWith("GET /start")){
              holdConstantTemp(30000, 94.0);  
          }
        }
        if (currentLine.endsWith("GET /start")) {
          holdConstantTemp(120000,94.0); // inital denaturing
          for (int i = 0; i < 21; i++){
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
