#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLE2902.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
#include <Adafruit_MPR121.h>

#include <Arduino.h>

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

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;


static inputConsumer_t consumer_Report{};
static inputKeyboard_t empty_keyboard_report{}; // sent to PC

BLEHIDDevice* hid;
BLECharacteristic* input;
BLECharacteristic* output;
BLECharacteristic* inputVolume;
BLECharacteristic* outputVolume;
bool connected = false;

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

void setup() {
    Serial.begin(9600);

    while (!Serial) { // needed to keep leonardo/micro from starting too fast!
        delay(10);
    }

    // Default address is 0x5A, if tied to 3.3V its 0x5B
    // If tied to SDA its 0x5C and if SCL then 0x5D
//    if (!cap.begin(0x5A)) {
//        Serial.println("MPR121 not found, check wiring?");
//        while (1);
//    }
    xTaskCreate(taskServer, "server", 20000, NULL, 5, NULL);
}

void loop() {
    delay(1000);
    blinds->sendCommand(BlindsCommandUp, BlindsChannel1);

}

//    static bool volDirUp = true;
    // put your main code here, to run repeatedly:

    // Get the currently touched pads
//    currtouched = cap.touched();
//
//    inputKeyboard_t a{};
//    for (uint8_t i=0; i<12; i++) {
//        // it if *is* touched and *wasnt* touched before, alert!
//        if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
//            Serial.print(i); Serial.println(" touched");
//            if (connected) {
////                digitalWrite(ledPin, HIGH);
////            a.Key = random(0x02,0x27);
//                a.Key[i%6] = 0x02 + i;
//                //   a.reportId = 0x02;
//                input->setValue((uint8_t*)&a,sizeof(a));
//                input->notify();
//            }
//        }
//        // if it *was* touched and now *isnt*, alert!
//        if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
//            Serial.print(i); Serial.println(" released");
//
//            if(connected) {
//
////                digitalWrite(ledPin, HIGH);
//                a.Key[i%6] = 0x0;
////                   a.reportId = 0x02;
//                input->setValue((uint8_t*)&a,sizeof(a));
//                input->notify();
//
////                input->setValue((uint8_t*)(&empty_keyboard_report), sizeof(empty_keyboard_report));
////                input->notify();
//            }
//        }
//    }
//    // reset our state
//    lasttouched = currtouched;
//}

