/*
 MIT License

 Copyright (c) 2023-2024 Kazuhito Suda

 This file is part of Let's FIWARE M5Stack

 https://github.com/lets-fiware/lets-fiware.m5stack

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

// Board manager URL: https://dl.espressif.com/dl/package_esp32_index.json
// Board manager: esp32 by Espressif Systems 2.0.6
// Board: M5Stack-ATOM
// Library manager: M5Atom by M5Statck version 0.1.0

#include <PubSubClient.h>
#include <WiFi.h>

#include <M5Atom.h>

WiFiClient gWifiClient;
PubSubClient gClient(gWifiClient);

// Configure parameters
const char* gEssid  = "ESSID";
const char* gPassword  = "ESSID_PASSWORD";
const char* gMqttServer = "MOSQUITTO.EXAMPLE.COM";
const char* gMqttUsername = "MQTT_USER";
const char* gMqttPassword = "MQTT_PASSOWRD";
const int gMqttPort = 1883;
const String gMqttCientId = "actuator001";
const char *gMqttTopicCmd = "/izqetq603ovjcqpx627e4eib1u/actuator001/cmd";
const char *gMqttTopicCmdExe = "/ul/izqetq603ovjcqpx627e4eib1u/actuator001/cmdexe";
const int gInterval = 2 * 1000; // milli seconds

void callback(char* topic, byte* payload, unsigned int length);
CRGB ledColor(uint8_t r, uint8_t g, uint8_t b);

void setup() {
    M5.begin(true, false, true);

    M5.dis.drawpix(0, ledColor(0, 0, 0));

    WiFi.mode(WIFI_STA);
    WiFi.begin(gEssid, gPassword);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
  
    gClient.setServer(gMqttServer, gMqttPort);
    gClient.setCallback(callback); 
}

void loop() {
    while (!gClient.connected()) {
        String clientId = gMqttCientId + String(random(0xffff), HEX);
        if (!gClient.connect(clientId.c_str(), gMqttUsername, gMqttPassword)) {
            Serial.print("Failed. state=");
            Serial.print(gClient.state());
            delay(3000);
        } else {
            Serial.println("subscribe");
            gClient.subscribe(gMqttTopicCmd);
        }
    }

    gClient.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
    static int count = 0;
    static const char *done= "actuator001@led|done";
    static const char *err= "actuator001@led|error";
    const char *status = err;

    count++;

    Serial.print(topic);
    Serial.print(" ");
    String msg(payload, length);
    Serial.println(msg);
    if (msg.endsWith("|on")) {
      M5.dis.drawpix(0, ledColor(20, 20, 20));
      status = done;
    } else if (msg.endsWith("|off")) {
      M5.dis.drawpix(0, ledColor(0, 0, 0));
      status = done;
    }
    gClient.publish(gMqttTopicCmdExe, status);
}

CRGB ledColor(uint8_t r, uint8_t g, uint8_t b) {
  return (CRGB)((r << 16) | (g << 8) | b);
}
