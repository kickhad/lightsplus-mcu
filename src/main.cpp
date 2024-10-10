/*
This example uses FreeRTOS softwaretimers as there is no built-in Ticker library
*/


#include <WiFi.h>
extern "C" {
	#include "freertos/FreeRTOS.h"
	#include "freertos/timers.h"
}
#include <AsyncMqttClient.h>

#define WIFI_SSID "iothell"
#define WIFI_PASSWORD "51teertsmoT"

#define MQTT_HOST IPAddress(91,121,93,94)
#define MQTT_PORT 1883
#include "secrets.h"
#include <FastLED.h>
#define NUM_LEDS 30
#define DATA_PIN 17 // Change this to the pin your LED strip is connected to
CRGB leds[NUM_LEDS];

void turnOnLEDs(int* intArray, int size) {
  // Turn off all LEDs
  FastLED.clear();

  // Turn on the specified LEDs
  for (int i = 0; i < size; i++) {
    if (intArray[i] >= 0 && intArray[i] < NUM_LEDS) {
      leds[intArray[i]] = CRGB::White;  // Change color as needed
    }
  }

  // Show the updated LED state
  FastLED.show();
}


AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;


// Helper function to find the length of a string
int string_length(const char* str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// Helper function to convert a substring to an integer
int string_to_int(const char* str, int start, int end) {
    int result = 0;
    for (int i = start; i < end; i++) {
        result = result * 10 + (str[i] - '0');
    }
    return result;
}

// Main function to parse the integer array
int* parse_int_array(const char* input, int* size) {
    int len = string_length(input);
    int* int_array = (int*)malloc(len * sizeof(int)); // Worst case: every character is a digit
    if (int_array == NULL) {
        *size = 0;
        return NULL;
    }

    int index = 0;
    int start = 0;
    bool in_number = false;

    for (int i = 0; i <= len; i++) {
        if (input[i] >= '0' && input[i] <= '9') {
            if (!in_number) {
                start = i;
                in_number = true;
            }
        } else {
            if (in_number) {
                int end = i;
                int_array[index++] = string_to_int(input, start, end);
                in_number = false;
            }
        }
    }

    *size = index;
    return int_array;
}



void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        connectToMqtt();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        xTimerStart(wifiReconnectTimer, 0);
        break;
    }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe("lightsplus-20241007/a2", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);
  mqttClient.publish("lightsplus-20241007/a2", 0, true, "test 1");
  Serial.println("Publishing at QoS 0");
  uint16_t packetIdPub1 = mqttClient.publish("lightsplus-20241007/a2", 1, true, "test 2");
  Serial.print("Publishing at QoS 1, packetId: ");
  Serial.println(packetIdPub1);
  uint16_t packetIdPub2 = mqttClient.publish("lightsplus-20241007/a2", 2, true, "test 3");
  Serial.print("Publishing at QoS 2, packetId: ");
  Serial.println(packetIdPub2);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");


  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}
void printArray(int* arr, int size) {
  for (int i = 0; i < size; i++) {
    Serial.println(arr[i]);
  }
}


void updateLEDs(const int* intArray, int size) {
  for (int i = 0; i < NUM_LEDS; i++) {
    bool ledShouldBeOn = false;
    for (int j = 0; j < size; j++) {
      if (i == intArray[j]) {
        ledShouldBeOn = true;
        break;
      }
    }
    leds[i] = ledShouldBeOn ? CRGB::Blue : CRGB::Black;
  }
  FastLED.show();
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
  int size;
  int* int_array = parse_int_array(payload, &size);
  updateLEDs(int_array,size);
  
  if (int_array != NULL) {
        for (int i = 0; i < size; i++) {
            Serial.print(int_array[i]);
            Serial.print(" ");
        }
        Serial.println();
        free(int_array);
  }


}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();
}

void loop() {
}


// #include <Arduino.h>
// #include <WiFi.h>
// #include <PubSubClient.h>

// WiFiClient espClient;
// PubSubClient client(espClient);

// // FastLED setup


// void setup_wifi() {
//   delay(10);
//   Serial.println();
//   Serial.print("Connecting to ");
//   Serial.println(ssid);

//   WiFi.begin(ssid, password);

//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }

//   Serial.println("");
//   Serial.println("WiFi connected");
//   Serial.println("IP address: ");
//   Serial.println(WiFi.localIP());
// }

// void turnOnLEDs(int* intArray, int size) {
//   // Turn off all LEDs
//   FastLED.clear();

//   // Turn on the specified LEDs
//   for (int i = 0; i < size; i++) {
//     if (intArray[i] >= 0 && intArray[i] < NUM_LEDS) {
//       leds[intArray[i]] = CRGB::White;  // Change color as needed
//     }
//   }

//   // Show the updated LED state
//   FastLED.show();
// }

// void callback(char* topic, byte* payload, unsigned int length) {
//   Serial.print("Message arrived [");
//   Serial.print(topic);
//   Serial.print("] ");

//   // Convert payload to a string
//   String message = "";
//   for (int i = 0; i < length; i++) {
//     message += (char)payload[i];
//   }
//   Serial.println(message);

//   // Split the message into an array of integers
//   int intArray[10]; // Adjust the size as needed
//   int index = 0;
//   char* token = strtok((char*)message.c_str(), ",");
//   while (token != NULL && index < 10) {
//     intArray[index++] = atoi(token);
//     token = strtok(NULL, ",");
//   }

//   // Print the integer array
//   Serial.print("Parsed integers: ");
//   for (int i = 0; i < index; i++) {
//     Serial.print(intArray[i]);
//     Serial.print(" ");
//   }
//   Serial.println();

//   // Turn on the LEDs based on the integer array
//   turnOnLEDs(intArray, index);
// }

// void reconnect() {
//   while (!client.connected()) {
//     Serial.print("Attempting MQTT connection...");
//     if (client.connect("ESP32Client")) {
//       Serial.println("connected");
//       client.subscribe(topic);
//       Serial.println(topic);
//     } else {
//       Serial.print("failed, rc=");
//       Serial.print(client.state());
//       Serial.println(" try again in 5 seconds");
//       delay(5000);
//     }
//   }
// }

// void setup() {
//   Serial.begin(115200);
//   setup_wifi();
//   client.setServer(mqtt_server, mqtt_port);
//   client.setCallback(callback);

//   // Initialize FastLED
//   FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
// }

// void loop() {
//   if (!client.connected()) {
//     reconnect();
//   }
//   client.loop();
// }