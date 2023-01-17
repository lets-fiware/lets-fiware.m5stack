/*
 MIT License

 Copyright (c) 2023 Kazuhito Suda

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
// Board: M5Stack-FIRE
// Library manager: M5Stack by M5Stack version 0.4.3
// Sensor: ENV III Unit (SHT30, QMP6988)

#include <M5Stack.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "time.h"
#include "M5_ENV.h"

WiFiClient gWifiClient;
PubSubClient gClient(gWifiClient);

SHT3X gSht30;
QMP6988 gQmp6988;

// Configure parameters
const char* gEssid = "ESSID";
const char* gPassword = "ESSID_PASSWORD";
const char* gMqttServer = "MOSQUITTO.EXAMPLE.COM";
const char* gMqttUsername = "MQTT_USER";
const char* gMqttPassword = "MQTT_PASSOWRD";
const int gMqttPort = 1883;
const String gMqttCientId = "sensor001-";
const char *gMqttTopic = "/izqetq603ovjcqpx627e4eib1u/sensor001/attrs";
const int gInterval = 2 * 1000; // milli seconds

#define MSG_MAX_SIZE (100)
char msg[MSG_MAX_SIZE];

void setup() {
    M5.begin();
    M5.Power.begin();
    M5.Lcd.setTextSize(2);

    Wire.begin();
    gQmp6988.init();

    WiFi.mode(WIFI_STA);
    WiFi.begin(gEssid, gPassword);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        M5.Lcd.print(".");
    }
  
    gClient.setServer(gMqttServer, gMqttPort);

    configTime(9 * 3600, 0, "ntp.jst.mfeed.ad.jp");    
}

void loop() {
    static int count = 0;
    static unsigned long previous = 0;    

    while (!gClient.connected()) {
        String clientId = gMqttCientId + String(random(0xffff), HEX);
        if (!gClient.connect(clientId.c_str(), gMqttUsername, gMqttPassword)) {
            M5.Lcd.setTextColor(RED, BLACK);
            M5.Lcd.print("Failed. state=");
            M5.Lcd.print(gClient.state());
            delay(3000);
        }
    }

    gClient.loop();

    unsigned long now = millis();
    if (now - previous > gInterval) {
        previous = now;
        ++count;

        struct tm timeinfo;
        getLocalTime(&timeinfo);

        float pressure  = gQmp6988.calcPressure() / 100;
        
        float temperature = 0.0, humidity = 0.0;
        if (gSht30.get() == 0) {
          temperature = gSht30.cTemp;                            
          humidity = gSht30.humidity;
        }

        snprintf(msg, MSG_MAX_SIZE,
                "d|%04d-%02d-%02dT%02d:%02d:%02d+0000|t|%2.1f|h|%2.1f|p|%2.1f",
                timeinfo.tm_year+1900,
                timeinfo.tm_mon+1,
                timeinfo.tm_mday,
                timeinfo.tm_hour,
                timeinfo.tm_min,
                timeinfo.tm_sec,
                temperature,
                humidity,
                pressure
              );
        gClient.publish(gMqttTopic, msg);

        M5.Lcd.fillScreen(BLACK);

        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.print("Temperature: ");
        M5.Lcd.setTextColor(GREEN, BLACK);
        M5.Lcd.print(String(temperature, 1));
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.print("c");

        M5.Lcd.setCursor(0, 20);
        M5.Lcd.print("Humidity: ");
        M5.Lcd.setTextColor(GREEN, BLACK);
        M5.Lcd.print(String(humidity, 1));
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.print("%");

        M5.Lcd.setCursor(0, 40);
        M5.Lcd.print("Pressure: ");
        M5.Lcd.setTextColor(GREEN, BLACK);
        M5.Lcd.print(String(pressure, 1));
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.print("hPa");

        M5.Lcd.setCursor(0, 60);
        M5.Lcd.print("Send: ");
        M5.Lcd.setTextColor(GREEN, BLACK);
        M5.Lcd.println(String(count));
    }
}
