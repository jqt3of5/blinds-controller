#include <WiFi.h>
#include <Arduino.h>

#include <ArduinoHA.h>
#include <ArduinoOTA.h>
#include <HACover.h>

#include "esp_task_wdt.h"

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

const char * ssid = "WaitingOnComcast";
const char * pwd = "1594N2640W";
const char * mqtt_host = "tiltpi.equationoftime.tech";
const int WDT_TIMEOUT = 5;

HADevice device("auto_watering");
WiFiClient client;
HAMqtt mqtt(client, device);

HASensor * controller_uptime = new HASensor("uptime");
HASensor * soil_moisture_sensor = new HASensor("moisture");
HASwitch * water_pump_switch = new HASwitch("pump",false);

int pump_pin = 30;
int moisture_sensor_pin = 30;

void setup() {

    esp_task_wdt_init(WDT_TIMEOUT, true);
    esp_task_wdt_add(NULL);

    Serial.begin(9600);

    while (!Serial) { // needed to keep leonardo/micro from starting too fast!
        delay(10);
    }

    WiFi.begin(ssid, pwd);
    WiFi.setAutoReconnect(true);

    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");
        delay(1000);
        ESP.restart();
    }

    // Port defaults to 3232
    // ArduinoOTA.setPort(3232);

    // Hostname defaults to esp3232-[MAC]
    // ArduinoOTA.setHostname("myesp32");

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    ArduinoOTA
            .onStart([]() {
                String type;
                if (ArduinoOTA.getCommand() == U_FLASH)
                    type = "sketch";
                else // U_SPIFFS
                    type = "filesystem";

                // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                Serial.println("Start updating " + type);
            })
            .onEnd([]() {
                Serial.println("\nEnd");
            })
            .onProgress([](unsigned int progress, unsigned int total) {
                Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
            })
            .onError([](ota_error_t error) {
                Serial.printf("Error[%u]: ", error);
                if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
                else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
                else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
                else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
                else if (error == OTA_END_ERROR) Serial.println("End Failed");
            });

    ArduinoOTA.begin();

    device.setName("Auto Waterer");
    device.enableSharedAvailability();
    device.setSoftwareVersion("0.0.1");
    device.enableLastWill();

    controller_uptime->setName("Auto Waterer Uptime");
    controller_uptime->setDeviceClass("duration");

    water_pump_switch->setName("Water Pump");

    soil_moisture_sensor->setName("Soil Moisture");
    soil_moisture_sensor->setDeviceClass("moisture");

    if (!mqtt.begin(mqtt_host, 1883))
    {
        Serial.println("Failed to connect to mqtt broker");
    }

    while (!mqtt.isConnected())
    {
        Serial.println("Connecting to mqtt");
        mqtt.loop();
        delay(500);
    }

    digitalWrite(pump_pin, 0);
}

uint32_t lastPublished = 0;
uint32_t pumpStarted = 0;
bool pumpRunning = false;
void loop() {

    esp_task_wdt_reset();

    mqtt.loop();
    ArduinoOTA.handle();

    if (millis() - lastPublished > 5000)
    {
        lastPublished = millis();
        controller_uptime->setValue(lastPublished);

        int moisture = analogRead(moisture_sensor_pin);
        soil_moisture_sensor->setValue(moisture);
    }

    //Shutdown the pump after 30 seconds. Meant as a safety to prevent flooding.
    if (pumpRunning && millis() - pumpStarted > 30000)
    {
        digitalWrite(pump_pin, 0);
        water_pump_switch->turnOff();
        pumpRunning = false;
    }

    if (water_pump_switch->getState())
    {
        if (!pumpRunning)
        {
            pumpStarted = millis();
            pumpRunning = true;
        }
        digitalWrite(pump_pin, 1);
    }
    else {
        digitalWrite(pump_pin, 0);
        pumpRunning = false;
    }
}