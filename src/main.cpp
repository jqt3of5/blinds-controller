//#include <BLEDevice.h>
//#include <BLEUtils.h>
//#include <BLEServer.h>
//#include "BLE2902.h"
//#include "BLEHIDDevice.h"
//#include "HIDTypes.h"
//#include "HIDKeyboardTypes.h"
#include <WiFi.h>

#include <Arduino.h>

#define ARDUINOHA_SWITCH
#include <ArduinoHA.h>
#include <ArduinoOTA.h>

#include "blinds.h"
//#include "hidReportDescriptor.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/i

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

// You can have up to 4 on one i2c bus but one is enough for testing!
//Adafruit_MPR121 cap = Adafruit_MPR121();

HADevice device("blinds_controller");

WiFiClient client;
HAMqtt mqtt(client, device);

const int channel_count = 5;
Blinds * blinds[] = {
    new Blinds(12, BlindsChannel1),
    new Blinds(12, BlindsChannel2),
    new Blinds(12, BlindsChannel3),
    new Blinds(12, BlindsChannel4),
    new Blinds(12, BlindsChannel5)
};

HACover * blindsHA [] = {
    new HACover("channel1", mqtt),
    new HACover("channel2", mqtt),
    new HACover("channel3", mqtt),
    new HACover("channel4", mqtt),
    new HACover("channel5",  mqtt)
};

int channel_leds[channel_count] = {2, 0, 0, 0, 0};

void commandHandler(Blinds * blinds, HACover * cover, HACover::CoverCommand command)
{
    noInterrupts();
    blinds->sendCommand(command);

    switch(command)
    {
        case HACover::CommandOpen:
            cover->setState(HACover::StateOpening);
            //TODO: A sensor of some kind would be awesome
            cover->setState(HACover::StateOpen);
            break;
        case HACover::CommandClose:
            cover->setState(HACover::StateClosing);
            //TODO: A sensor of some kind would be awesome
            cover->setState(HACover::StateClosed);
            break;
        case HACover::CommandStop:
            cover->setState(HACover::StateStopped);
            break;
    }
    interrupts();
}

template<int n> void blindsChannel(HACover::CoverCommand command)
{
    commandHandler(blinds[n], blindsHA[n], command);
}

void (*blindsCommandHandler [])(HACover::CoverCommand cmd) = {
        blindsChannel<0>,
        blindsChannel<1>,
        blindsChannel<2>,
        blindsChannel<3>,
        blindsChannel<4>,
};

void setup() {
    Serial.begin(9600);

    while (!Serial) { // needed to keep leonardo/micro from starting too fast!
        delay(10);
    }

    WiFi.begin("WaitingOnComcast", "1594N2640W");
    WiFi.setAutoReconnect(true);

    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
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

    device.setName("Blinds Controller");
    device.enableSharedAvailability();
    device.setSoftwareVersion("0.9.2");
    device.enableLastWill();

    for (int i = 0; i < channel_count; ++i)
    {
        char * name = (char*)calloc(32, 1);
        sprintf(name, "Blinds Channel %d", i);
        blindsHA[i]->setName(name);
        blindsHA[i]->onCommand(blindsCommandHandler[i]);
    }

    if (!mqtt.begin("tiltpi.equationoftime.tech", 1883))
    {
        Serial.println("Failed to connect to mqtt broker");
    }

    while (!mqtt.isConnected())
    {
        Serial.println("Connecting to mqtt");
        mqtt.loop();
        delay(500);
    }

    for (int i = 0; i < channel_count; ++i)
    {
        blindsHA[i]->setState(HACover::StateUnknown, true);
        blindsHA[i]->setPosition(50);
        blindsHA[i]->setAvailability(true);

        pinMode(channel_leds[i], OUTPUT);
    }

    digitalWrite(channel_leds[0], HIGH);

}

int currentChannel = 0;

void loop() {
    mqtt.loop();
    ArduinoOTA.handle();

    //If up/down/stop button tapped
    if (false) {
        blindsCommandHandler[currentChannel](HACover::CommandOpen);
    }

    //If channel select
    if (false) {
        currentChannel = (currentChannel + 1) % channel_count;
        currentChannel = (currentChannel + channel_count - 1) % channel_count;
    }

    for (int i = 0; i < channel_count; ++i)
    {
        digitalWrite(channel_leds[i], i == currentChannel ? HIGH : LOW);
    }
}