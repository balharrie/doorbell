//
// MIT License
//
// Copyright (c) 2020 Neil Balharrie
//
// Listens for a signal from a 433MHz receiver unit and sends a message to a MQTT broker.
//

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// RCSwitch library required for managing 433MHz signals from the doorbell.
#include "RCSwitch.h"

#include "arduino_secrets.h"

// 433Mhz receiver is attached to pin D2 (interrupt 0)
#define RX433MHZ_PIN D2

#define DBG_OUTPUT_PORT Serial

const char *mqtt_broker = SECRETS_MQTT_BROKER_IP;
const int mqtt_port = SECRETS_MQTT_BROKER_PORT;

const char *newWifiSSID = SECRETS_WIFI_SSID;
const char *newWifiPasskey = SECRETS_WIFI_PASSWORD;

const char *hostName = "doorbell";

RCSwitch mySwitch = RCSwitch();

WiFiClient espClient;
PubSubClient client(espClient);

/**
 * Useful funtion to blink the D1 MINI LED
 */
void blink() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
}

/**
 * Ensure the broker connection is made.
 */
void connectToBroker() {
  while (!client.connected()) {
    blink();
    if (client.connect("esp8266-client")) {
      DBG_OUTPUT_PORT.println("Connected");
    }
    else {
      DBG_OUTPUT_PORT.print("Failed with state ");
      DBG_OUTPUT_PORT.print(client.state());
      delay(2000);
    }
  }
}

/**
 * Ensure the WI-FI connection is running
 */
void connectWiFi() {
  DBG_OUTPUT_PORT.print("Connecting to WiFi..");
  WiFi.enableAP(false);
  WiFi.hostname(hostName);
  WiFi.begin(newWifiSSID, newWifiPasskey);
  WiFi.setAutoReconnect(true);
  while (WiFi.status() != WL_CONNECTED) {
    DBG_OUTPUT_PORT.print(".");
    blink();
  }
  DBG_OUTPUT_PORT.println();
  DBG_OUTPUT_PORT.println("Connected to the WiFi network");
}

void setup() {
  // Serial port up
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.setDebugOutput(false);
  
  // Enable the built-in LED
  pinMode(LED_BUILTIN, OUTPUT);

  // Bring up the WI-FI
  connectWiFi();

  // Connect broker
  DBG_OUTPUT_PORT.println("Connecting to broker.....");
  client.setServer(mqtt_broker, mqtt_port);
  connectToBroker();

  // Enable 433MHz receiver for 1byone doorbells
  mySwitch.setProtocol(10);
  mySwitch.enableReceive(RX433MHZ_PIN);
  DBG_OUTPUT_PORT.println("Listening for 433MHz signals...");
}

void debugOutputReceivedCode() {
  // Print to Serial information about the signal we found (Use the first value for checking received codes)
  // Uncomment and use this function if you are interested in the code for your doorbell
  // Serial.print("Signal Received:  ");
  // Serial.print("Value: "); Serial.print(mySwitch.getReceivedValue() );
  // Serial.print(" / "); Serial.print(mySwitch.getReceivedValue(), BIN);
  // Serial.print(" Length: "); Serial.print(mySwitch.getReceivedBitlength() );
  // Serial.print(" Delay: "); Serial.print(mySwitch.getReceivedDelay() );
  // Serial.print(" Protocol: "); Serial.println(mySwitch.getReceivedProtocol());
}

long lastMicros = 0L;

void loop() {
  if (mySwitch.available()) {
    blink();
    if (mySwitch.getReceivedValue() == 117983 && mySwitch.getReceivedBitlength() == 17 /*&& mySwitch.getReceivedDelay() == 362*/ && mySwitch.getReceivedProtocol() == 10) {
      long now = micros();
      if (lastMicros == 0 || now - lastMicros > 600000) {
        DBG_OUTPUT_PORT.print("Doorbell pressed ");
        DBG_OUTPUT_PORT.println(now - lastMicros);
        connectToBroker();
        client.publish("doorbell/ring", "1");
      }
      lastMicros = now;
    }

    mySwitch.resetAvailable();
  }
}
