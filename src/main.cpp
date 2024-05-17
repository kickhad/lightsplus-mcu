#include <main.h>
#include <SPI.h>
#include "SPIFFS.h"
// For Wifi Server
#include <WiFi.h>
char ssid[] = "";  // your network SSID (name)

char pass[] = "";  // your network passwords


// For LED strips
#include <FastLED.h>
#include <ESPAsyncWebServer.h>

// char ssid[] = "";  // your network SSID (name)
// char pass[] = "";  // your network password

int keyIndex = 0;  // your network key Index number (needed only for WEP)
int currentLED = 0;
#define LED_PIN 4
#define LED_PIN2 15
#define NUM_LEDS 1500
int status = WL_IDLE_STATUS;
CRGB leds[NUM_LEDS];
CRGB leds2[NUM_LEDS];

//WiFiServer server(80);
AsyncWebServer server(80);

void setup() {

  setupVGA();
  //Initialize serial and wait for port to open:

  Serial.begin(9600);
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:

    status = WiFi.begin(ssid, pass);
    status = WiFi.waitForConnectResult();

    // wait 10 seconds for connection:
    Serial.print("Status was ");
    Serial.println(status);

    delay(5000);
  }

  setupRoutes();
  startServer();

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2812, LED_PIN2, GRB>(leds2, NUM_LEDS);
  for (int i = 0; i <= NUM_LEDS - 1; i++) {
    leds[i] = CRGB(0, 0, 0);
    leds2[i] = CRGB(0, 0, 0);
  }
  FastLED.show();

  printWifiStatus();
}


void loop() {


  // listen for incoming clients


  webserverstuff();
  // for (int i = 0; i <= NUM_LEDS  -1 ; i++) {
  // if (currentLED == 0)
  // {
  //   leds[i] = CRGB ( 16, 0, 0);
  //   leds2[i] = CRGB ( 16, 0, 0);
  // }
  // else if (currentLED == 1)
  // {
  //   leds[i] = CRGB ( 0, 255, 0);
  //   leds2[i] = CRGB ( 0, 255, 0);

  // }
  // else if (currentLED == 2)
  // {
  //   leds[i] = CRGB ( 0, 0, 255);
  //   leds2[i] = CRGB ( 0, 0, 255);

  // }
// }
//FastLED.show();

// currentLED++;
// if (currentLED == 3)
//   currentLED = 0;

delay(100);
// Serial.println("client disconnected");
}

void webserverstuff() {
}

void handleRoot(AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "Hello from ESP32 Board Game Light Controller!");
}

void handleReset(AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "All lights turning off.");
  for (int i = 0; i <= NUM_LEDS - 1; i++) {
    leds[i] = CRGB(0, 0, 0);
    leds2[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
  request->send(200, "text/plain", "All lights off!");
}

void handleListData(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Parsing list data");
    // Take post data and dump in file

    request->send(200, "text/plain", "List data finished.");
}

void handleApp(AsyncWebServerRequest *request) {
  // Send HTML file with javascript
  request->send(SPIFFS, "/index.html", String());
}


void setupRoutes() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/reset", HTTP_GET, handleReset);
  server.on("/postlistdata", HTTP_POST, handleListData);
  server.on("/app", HTTP_GET, handleApp);
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });  
  // Put this routing somewhere else
  server.on("/lightset", HTTP_GET, [](AsyncWebServerRequest *request) {
    int paramsNr = request->params();
    Serial.println(paramsNr);

    for (int i = 0; i < paramsNr; i++) {

      AsyncWebParameter *p = request->getParam(i);
      Serial.println(p->value());
      if (p->name() == "on") {
        leds[p->value().toInt()] = CRGB(255, 255, 255);
        Serial.println("Turned on");
        FastLED.show();
      }
      if (p->name() == "off") {
        leds[p->value().toInt()] = CRGB(0, 0, 0);
        Serial.println("Turned off");
        FastLED.show();
      }
      // Test code for params
      // Serial.print("Param name: ");
      // Serial.println(p->name());
      // Serial.print("Param value: ");
      // Serial.println(p->value());
      // Serial.println("------");
    }

    request->send(200, "text/plain", "message received");
  });
}

void startServer() {
  server.begin();
  Serial.println("Server started");
}
void printWifiStatus() {


  // print the SSID of the network you're attached to:


  Serial.print("SSID: ");


  Serial.println(WiFi.SSID());


  // print your WiFi shield's IP address:


  IPAddress ip = WiFi.localIP();


  Serial.print("IP Address: ");


  Serial.println(ip);
  // Send it to the VGA screen
  PrintIPtoVGA(ip);

  // print the received signal strength:


  long rssi = WiFi.RSSI();


  Serial.print("signal strength (RSSI):");


  Serial.print(rssi);


  Serial.println(" dBm");
}

void setupVGA()
{


}
void PrintIPtoVGA(IPAddress ip)
{

}
