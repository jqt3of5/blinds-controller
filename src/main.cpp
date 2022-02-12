#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLE2902.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
#include <Adafruit_MPR121.h>
#include <WiFi.h>

#include <Ethernet.h>
#include <Arduino.h>
#include <device-types/HABinarySensor.h>
#include <ArduinoHA.h>

#include "blinds.h"
#include "hidReportDescriptor.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/i

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();
Blinds * blinds = new Blinds(12);

int switch_count = 5;
HASwitch * switches [] = {
new HASwitch("blinds_channel1", true),
new HASwitch("blinds_channel2", true),
new HASwitch("blinds_channel3", true),
new HASwitch("blinds_channel4", true),
new HASwitch("blinds_channel5", true)
};

HADevice device("blinds_touch_pad");
EthernetClient client;
HAMqtt mqtt(client, device);

static inputConsumer_t consumer_Report{};
static inputKeyboard_t empty_keyboard_report{}; // sent to PC

BLEHIDDevice* hid;
BLECharacteristic* input;
BLECharacteristic* output;
BLECharacteristic* inputVolume;
BLECharacteristic* outputVolume;
bool connected = false;

char  ha_token[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiIzNzhjMDUyYzViZDU0NDgwYmRlZmJiMGVlYzM4YTEyOSIsImlhdCI6MTY0NDYzODYxOCwiZXhwIjoxOTU5OTk4NjE4fQ.0iUaKd_cjfLZLwEp3WXUgbx2TV-Y7bGXT-lDy09WW1U";

class MyCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer){
        connected = true;
        BLE2902* desc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
        desc->setNotifications(true);

        BLE2902* descv = (BLE2902*)inputVolume->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
        descv->setNotifications(true);
    }

    void onDisconnect(BLEServer* pServer){
        connected = false;
        BLE2902* desc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
        desc->setNotifications(false);

        BLE2902* descv = (BLE2902*)inputVolume->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
        descv->setNotifications(false);
    }
};

void taskServer(void*){
    BLEDevice::init("TouchPad");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyCallbacks());

    hid = new BLEHIDDevice(pServer);
    inputVolume = hid->inputReport(1); // <-- input REPORTID from report map
    outputVolume = hid->outputReport(1); // <-- output REPORTID from report map

    input = hid->inputReport(2); // <-- input REPORTID from report map
    output = hid->outputReport(2); // <-- output REPORTID from report map

    std::string name = "SubstantiveTech";
    hid->manufacturer()->setValue(name);

    hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
    hid->hidInfo(0x00,0x02);

    hid->reportMap((uint8_t*)keyboardHidDescriptor, sizeof(keyboardHidDescriptor));
    hid->startServices();


    BLESecurity *pSecurity = new BLESecurity();
//  pSecurity->setKeySize();
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);


    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->setAppearance(HID_KEYBOARD);
    pAdvertising->addServiceUUID(hid->hidService()->getUUID());
    pAdvertising->start();
    hid->setBatteryLevel(98);

    delay(portMAX_DELAY);
}

void onBlindsStateChanged(bool state, HASwitch * s)
{
    for(int i = 0; i < switch_count; ++i)
    {
        if (s == switches[i])
        {
            switch(i + 1)
            {
                case 1:
                    blinds->sendCommand(state ? BlindsCommandOpen : BlindsCommandClose, BlindsChannel1);
                   break;
                case 2:
                    blinds->sendCommand(state ? BlindsCommandOpen : BlindsCommandClose, BlindsChannel2);
                    break;
                case 3:
                    blinds->sendCommand(state ? BlindsCommandOpen : BlindsCommandClose, BlindsChannel3);
                    break;
                case 4:
                    blinds->sendCommand(state ? BlindsCommandOpen : BlindsCommandClose, BlindsChannel4);
                    break;
                case 5:
                    blinds->sendCommand(state ? BlindsCommandOpen : BlindsCommandClose, BlindsChannel5);
                    break;
            }
        }
    }

}

void setup() {
    Serial.begin(9600);

    while (!Serial) { // needed to keep leonardo/micro from starting too fast!
        delay(10);
    }

    WiFi.begin("WaitingOnComcast", "1594N2640W");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }

    device.setName("Blinds TouchPad");
    device.setSoftwareVersion("0.0.1");

    char name[16] = {0};
    for (int i = 0; i < switch_count; ++i)
    {
        sprintf(name, "Blinds Channel %d", i);
       switches[i]->setName(name);

       //TODO: Find a better icon
       switches[i]->setIcon("mdi:lightbulb");
       switches[i]->onStateChanged(onBlindsStateChanged);
    }

    //TODO: Does thebrokerhave a username andpassword?

    mqtt.begin("tiltpi.equationoftime.tech:1883");

    // Default address is 0x5A, if tied to 3.3V its 0x5B
    // If tied to SDA its 0x5C and if SCL then 0x5D
//    if (!cap.begin(0x5A)) {
//        Serial.println("MPR121 not found, check wiring?");
//        while (1);
//    }
    xTaskCreate(taskServer, "server", 20000, NULL, 5, NULL);
}

void loop() {
    mqtt.loop();

//    blinds->sendCommand(BlindsCommandUp, BlindsChannel1);

//    inputKeyboard_t a{};
//    a.Key[i%6] = 0x02 + i;
//                //   a.reportId = 0x02;
//                input->setValue((uint8_t*)&a,sizeof(a));
//                input->notify();
}