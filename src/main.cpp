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

#include "blinds.h"
//#include "hidReportDescriptor.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/i

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

// You can have up to 4 on one i2c bus but one is enough for testing!
//Adafruit_MPR121 cap = Adafruit_MPR121();

Blinds * blinds[] = {
new Blinds(12, BlindsChannel1),
new Blinds(12, BlindsChannel2),
new Blinds(12, BlindsChannel3),
new Blinds(12, BlindsChannel4),
new Blinds(12, BlindsChannel5)
};
//

HADevice device("blinds_touch_pad");
WiFiClient client;
HAMqtt mqtt(client, device);

int switch_count = 5;
HACover * blindsHA [] = {
new HACover("blinds_channel1", mqtt),
new HACover("blinds_channel2", mqtt),
new HACover("blinds_channel3", mqtt),
new HACover("blinds_channel4", mqtt),
new HACover("blinds_channel5",  mqtt)
};

//static inputConsumer_t consumer_Report{};
//static inputKeyboard_t empty_keyboard_report{}; // sent to PC

//BLEHIDDevice* hid;
//BLECharacteristic* input;
//BLECharacteristic* output;
//BLECharacteristic* inputVolume;
//BLECharacteristic* outputVolume;
//bool connected = false;

//char  ha_token[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiIzNzhjMDUyYzViZDU0NDgwYmRlZmJiMGVlYzM4YTEyOSIsImlhdCI6MTY0NDYzODYxOCwiZXhwIjoxOTU5OTk4NjE4fQ.0iUaKd_cjfLZLwEp3WXUgbx2TV-Y7bGXT-lDy09WW1U";
//
//class MyCallbacks : public BLEServerCallbacks {
//    void onConnect(BLEServer* pServer){
//        connected = true;
//        BLE2902* desc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
//        desc->setNotifications(true);
//
//        BLE2902* descv = (BLE2902*)inputVolume->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
//        descv->setNotifications(true);
//    }
//
//    void onDisconnect(BLEServer* pServer){
//        connected = false;
//        BLE2902* desc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
//        desc->setNotifications(false);
//
//        BLE2902* descv = (BLE2902*)inputVolume->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
//        descv->setNotifications(false);
//    }
//};
//
//void taskServer(void*){
//    BLEDevice::init("TouchPad");
//    BLEServer *pServer = BLEDevice::createServer();
//    pServer->setCallbacks(new MyCallbacks());
//
//    hid = new BLEHIDDevice(pServer);
//    inputVolume = hid->inputReport(1); // <-- input REPORTID from report map
//    outputVolume = hid->outputReport(1); // <-- output REPORTID from report map
//
//    input = hid->inputReport(2); // <-- input REPORTID from report map
//    output = hid->outputReport(2); // <-- output REPORTID from report map
//
//    std::string name = "SubstantiveTech";
//    hid->manufacturer()->setValue(name);
//
//    hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
//    hid->hidInfo(0x00,0x02);
//
//    hid->reportMap((uint8_t*)keyboardHidDescriptor, sizeof(keyboardHidDescriptor));
//    hid->startServices();
//
//
//    BLESecurity *pSecurity = new BLESecurity();
////  pSecurity->setKeySize();
//    pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);
//
//
//    BLEAdvertising *pAdvertising = pServer->getAdvertising();
//    pAdvertising->setAppearance(HID_KEYBOARD);
//    pAdvertising->addServiceUUID(hid->hidService()->getUUID());
//    pAdvertising->start();
//    hid->setBatteryLevel(98);
//
//    delay(portMAX_DELAY);
//}
//

void commandHandler(Blinds * blinds, HACover * cover, HACover::CoverCommand command)
{
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

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }

    device.setName("Blinds TouchPad");
    device.setSoftwareVersion("0.0.1");
    device.enableSharedAvailability();
    device.setAvailability(true);

    for (int i = 0; i < switch_count; ++i)
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

    for (int i = 0; i < switch_count; ++i)
    {
        blindsHA[i]->setState(HACover::StateUnknown, true);
        blindsHA[i]->setPosition(50);
        blindsHA[i]->setAvailability(true);
    }
//
    // Default address is 0x5A, if tied to 3.3V its 0x5B
    // If tied to SDA its 0x5C and if SCL then 0x5D
//    if (!cap.begin(0x5A)) {
//        Serial.println("MPR121 not found, check wiring?");
//        while (1);
//    }
//    xTaskCreate(taskServer, "server", 20000, NULL, 5, NULL);
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