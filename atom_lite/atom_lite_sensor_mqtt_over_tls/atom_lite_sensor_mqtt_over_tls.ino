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
// Sensor: ENV III Unit (SHT30, QMP6988)

#include <M5Atom.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "time.h"
#include "M5_ENV.h"

WiFiClientSecure gWifiClient;
PubSubClient gClient(gWifiClient);

SHT3X gSht30;
QMP6988 gQmp6988;

// Configure parameters
const char* gEssid = "ESSID";
const char* gPassword = "ESSID_PASSWORD";
const char* gMqttServer = "MOSQUITTO.EXAMPLE.COM";
const char* gMqttUsername = "MQTT_USER";
const char* gMqttPassword = "MQTT_PASSOWRD";
const int gMqttPort = 8883;
const String gMqttCientId = "sensor001-";
const char *gMqttTopic = "/izqetq603ovjcqpx627e4eib1u/sensor001/attrs";
const int gInterval = 2 * 1000; // milli seconds

// openssl s_client -connect mosquitto.example.com:8883 -showcerts
//
// 2 s:C = US, O = Internet Security Research Group, CN = ISRG Root X1
//   i:O = Digital Signature Trust Co., CN = DST Root CA X3
//   a:PKEY: rsaEncryption, 4096 (bit); sigalg: RSA-SHA256
//   v:NotBefore: Jan 20 19:14:03 2021 GMT; NotAfter: Sep 30 18:14:03 2024 GMT

const char* gRootCa = R"(
-----BEGIN CERTIFICATE-----
MIIFYDCCBEigAwIBAgIQQAF3ITfU6UK47naqPGQKtzANBgkqhkiG9w0BAQsFADA/
MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT
DkRTVCBSb290IENBIFgzMB4XDTIxMDEyMDE5MTQwM1oXDTI0MDkzMDE4MTQwM1ow
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwggIiMA0GCSqGSIb3DQEB
AQUAA4ICDwAwggIKAoICAQCt6CRz9BQ385ueK1coHIe+3LffOJCMbjzmV6B493XC
ov71am72AE8o295ohmxEk7axY/0UEmu/H9LqMZshftEzPLpI9d1537O4/xLxIZpL
wYqGcWlKZmZsj348cL+tKSIG8+TA5oCu4kuPt5l+lAOf00eXfJlII1PoOK5PCm+D
LtFJV4yAdLbaL9A4jXsDcCEbdfIwPPqPrt3aY6vrFk/CjhFLfs8L6P+1dy70sntK
4EwSJQxwjQMpoOFTJOwT2e4ZvxCzSow/iaNhUd6shweU9GNx7C7ib1uYgeGJXDR5
bHbvO5BieebbpJovJsXQEOEO3tkQjhb7t/eo98flAgeYjzYIlefiN5YNNnWe+w5y
sR2bvAP5SQXYgd0FtCrWQemsAXaVCg/Y39W9Eh81LygXbNKYwagJZHduRze6zqxZ
Xmidf3LWicUGQSk+WT7dJvUkyRGnWqNMQB9GoZm1pzpRboY7nn1ypxIFeFntPlF4
FQsDj43QLwWyPntKHEtzBRL8xurgUBN8Q5N0s8p0544fAQjQMNRbcTa0B7rBMDBc
SLeCO5imfWCKoqMpgsy6vYMEG6KDA0Gh1gXxG8K28Kh8hjtGqEgqiNx2mna/H2ql
PRmP6zjzZN7IKw0KKP/32+IVQtQi0Cdd4Xn+GOdwiK1O5tmLOsbdJ1Fu/7xk9TND
TwIDAQABo4IBRjCCAUIwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYw
SwYIKwYBBQUHAQEEPzA9MDsGCCsGAQUFBzAChi9odHRwOi8vYXBwcy5pZGVudHJ1
c3QuY29tL3Jvb3RzL2RzdHJvb3RjYXgzLnA3YzAfBgNVHSMEGDAWgBTEp7Gkeyxx
+tvhS5B1/8QVYIWJEDBUBgNVHSAETTBLMAgGBmeBDAECATA/BgsrBgEEAYLfEwEB
ATAwMC4GCCsGAQUFBwIBFiJodHRwOi8vY3BzLnJvb3QteDEubGV0c2VuY3J5cHQu
b3JnMDwGA1UdHwQ1MDMwMaAvoC2GK2h0dHA6Ly9jcmwuaWRlbnRydXN0LmNvbS9E
U1RST09UQ0FYM0NSTC5jcmwwHQYDVR0OBBYEFHm0WeZ7tuXkAXOACIjIGlj26Ztu
MA0GCSqGSIb3DQEBCwUAA4IBAQAKcwBslm7/DlLQrt2M51oGrS+o44+/yQoDFVDC
5WxCu2+b9LRPwkSICHXM6webFGJueN7sJ7o5XPWioW5WlHAQU7G75K/QosMrAdSW
9MUgNTP52GE24HGNtLi1qoJFlcDyqSMo59ahy2cI2qBDLKobkx/J3vWraV0T9VuG
WCLKTVXkcGdtwlfFRjlBz4pYg1htmf5X6DYO8A4jqv2Il9DjXA6USbW1FzXSLr9O
he8Y4IWS6wY7bCkjCWDcRQJMEhg76fsO3txE+FiYruq9RUWhiF1myv4Q6W+CyBFC
Dfvp7OOGAN6dEOM4+qR9sdjoSYKEBpsr6GtPAQw4dy753ec5
-----END CERTIFICATE-----
)";

#define MSG_MAX_SIZE (100)
char msg[MSG_MAX_SIZE];

void setup() {
    M5.begin(true, false, true);

    Wire.begin();
    gQmp6988.init();

    WiFi.mode(WIFI_STA);
    WiFi.begin(gEssid, gPassword);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
  
    gWifiClient.setCACert(gRootCa);
    gClient.setServer(gMqttServer, gMqttPort);

    configTime(9 * 3600, 0, "ntp.jst.mfeed.ad.jp");    
}

void loop() {
    static int count = 0;
    static unsigned long previous = 0;    

    while (!gClient.connected()) {
        String clientId = gMqttCientId + String(random(0xffff), HEX);
        if (!gClient.connect(clientId.c_str(), gMqttUsername, gMqttPassword)) {
            Serial.print("Failed. state=");
            Serial.print(gClient.state());
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

        Serial.print(gMqttTopic); 
        Serial.print(" ");
        Serial.println(msg);
    }
}
