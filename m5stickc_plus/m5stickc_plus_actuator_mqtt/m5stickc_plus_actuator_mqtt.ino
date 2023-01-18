/*
 MIT License

 Copyright (c) 2023 Kazuhito Suda

 This file is part of Let's FIWARE M5Stack

 https://github.com/lets-MQTT_USER/lets-fiware.m5stack

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

// Board manager URL: https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json
// Board manager: M5Stack by M5Stack 2.0.6
// Board: M5Stick_C_Plus
// Library manager: M5StickCPlus by M5Stack version 0.0.8

#include <M5StickCPlus.h>
#include <PubSubClient.h>
#include <WiFi.h>

WiFiClient gWifiClient;
PubSubClient gClient(gWifiClient);

// Configure parameters
const char* gEssid = "ESSID";
const char* gPassword = "ESSID_PASSWORD";
const char* gMqttServer = "MOSQUITTO.EXAMPLE.COM";
const char* gMqttUsername = "MQTT_USER";
const char* gMqttPassword = "MQTT_PASSOWRD";
const int gMqttPort = 1883;
const String gMqttCientId = "actuator001";
const char *gMqttTopicCmd = "/izqetq603ovjcqpx627e4eib1u/actuator001/cmd";
const char *gMqttTopicCmdExe = "/ul/izqetq603ovjcqpx627e4eib1u/actuator001/cmdexe";
const int gInterval = 2 * 1000; // milli seconds

void callback(char* topic, byte* payload, unsigned int length);

void setup() {
    M5.begin();
    M5.Lcd.setRotation(3);
    M5.Lcd.setTextSize(2);

    pinMode(GPIO_NUM_10, OUTPUT);
    digitalWrite(GPIO_NUM_10, HIGH);

    WiFi.mode(WIFI_STA);
    WiFi.begin(gEssid, gPassword);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        M5.Lcd.print(".");
    }
  
    gClient.setServer(gMqttServer, gMqttPort);
    gClient.setCallback(callback); 
}

void loop() {
    while (!gClient.connected()) {
        String clientId = gMqttCientId + String(random(0xffff), HEX);
        if (!gClient.connect(clientId.c_str(), gMqttUsername, gMqttPassword)) {
            M5.Lcd.setTextColor(RED, BLACK);
            M5.Lcd.print("Failed. state=");
            M5.Lcd.print(gClient.state());
            delay(3000);
        } else {
            M5.Lcd.print("subscribe");
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

    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.print(String(count)+":");
    M5.Lcd.print(topic);
    String msg(payload, length);
    M5.Lcd.print(" " + msg);
    M5.Lcd.println();
    if (msg.endsWith("|on")) {
      digitalWrite(GPIO_NUM_10, LOW);
      status = done;
    } else if (msg.endsWith("|off")) {
      digitalWrite(GPIO_NUM_10, HIGH);
      status = done;
    }
    gClient.publish(gMqttTopicCmdExe, status);
}
