# Arduino 433Mhz Doorbell Gateway

Provides a simple interface between a cheap 1byOne 433Mhz doorbell to MQTT.

## arduino_secrets.h

In order to compile, you will need a `arduino_secrets.h` file alongside `Doorbell.cpp` containing the following defines:

* SECRETS_MQTT_BROKER_IP
* SECRETS_BROKER_PORT
* SECRETS_WIFI_SSID
* SECRETS_WIFI_PASSWORD

## Platform IO

Repository is configured to use D1 MINI board with Arduino environment.
