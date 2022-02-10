#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLE2902.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"

#include "HIDKeyboardTypes.h"
#include <Adafruit_MPR121.h>

#include <Arduino.h>
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/i

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

//https://www.usb.org/sites/default/files/hut1_22.pdf
const uint8_t keyboardHidDescriptor[] = {
        0x05, 0x0c,                    // USAGE_PAGE (Consumer Devices)
        0x09, 0x01,                    // USAGE (Consumer Control)
        0xa1, 0x01,                    // COLLECTION (Application)
        0x85, 0x01,                    //   REPORT_ID (1)
        0x19, 0x00,                    //   USAGE_MINIMUM (Unassigned)
        0x2a, 0x3c, 0x02,              //   USAGE_MAXIMUM (AC Format)
        0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
        0x26, 0x3c, 0x02,              //   LOGICAL_MAXIMUM (572)
        0x95, 0x01,                    //   REPORT_COUNT (1)
        0x75, 0x10,                    //   REPORT_SIZE (16)
        0x81, 0x00,                    //   INPUT (Data,Var,Abs)
        0xc0,                          // END_COLLECTION


        0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
        0x09, 0x07,                    // USAGE (Keypad)
        0xa1, 0x01,                    // COLLECTION (Application)
        0x85, 0x02,                    //   REPORT_ID (2)
        0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
        0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
        0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
        0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
        0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
        0x75, 0x01,                    //   REPORT_SIZE (1)
        0x95, 0x08,                    //   REPORT_COUNT (8)
        0x81, 0x02,                    //   INPUT (Data,Var,Abs)
        0x95, 0x01,                    //   REPORT_COUNT (1)
        0x75, 0x08,                    //   REPORT_SIZE (8)
        0x81, 0x01,                    //   INPUT (Constant) Reserved Byte
        0x95, 0x06,                    //   REPORT_COUNT (6)
        0x75, 0x08,                    //   REPORT_SIZE (8)
        0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
        0x05, 0x07,                    //   Usage Page (Key Codes)
        0x19, 0x00,                    //   Usage Minimum (Reserved (No event indicated))
        0x29, 0x65,                    //   Usage Minimum (Keyboard application)
        0x81, 0x00,                    //   INPUT (Data,Array) Key Arrays (6 bytes)
//        0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
//        0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
//        0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
        0xc0                           // END_COLLECTION
};


typedef struct
{
    //uint8_t  reportId;                                 // Report ID = 0x02 (2)
    // Collection: CA:ConsumerControl
    uint16_t ConsumerControl;                          // Value = 0 to 572
} inputConsumer_t;

static uint8_t idleRate;           /* in 4 ms units */

typedef struct
{
// uint8_t  reportId;                                 // Report ID = 0x02 (2)
    // Collection: CA:Keyboard
    uint8_t  KB_KeyboardKeyboardLeftControl  : 1;       // Usage 0x000700E0: Keyboard Left Control, Value = 0 to 1
    uint8_t  KB_KeyboardKeyboardLeftShift  : 1;         // Usage 0x000700E1: Keyboard Left Shift, Value = 0 to 1
    uint8_t  KB_KeyboardKeyboardLeftAlt    : 1;           // Usage 0x000700E2: Keyboard Left Alt, Value = 0 to 1
    uint8_t  KB_KeyboardKeyboardLeftGui    : 1;           // Usage 0x000700E3: Keyboard Left GUI, Value = 0 to 1
    uint8_t  KB_KeyboardKeyboardRightControl : 1;      // Usage 0x000700E4: Keyboard Right Control, Value = 0 to 1
    uint8_t  KB_KeyboardKeyboardRightShift   : 1;        // Usage 0x000700E5: Keyboard Right Shift, Value = 0 to 1
    uint8_t  KB_KeyboardKeyboardRightAlt   : 1;          // Usage 0x000700E6: Keyboard Right Alt, Value = 0 to 1
    uint8_t  KB_KeyboardKeyboardRightGui   : 1;          // Usage 0x000700E7: Keyboard Right GUI, Value = 0 to 1
    uint8_t  _reserved;
    uint8_t  Key[6];                                 // Value = 0 to 101
} inputKeyboard_t;

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


const int ledPin = A12;


void taskServer(void*){
    BLEDevice::init("ESP32");
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
    hid->setBatteryLevel(7);

    delay(portMAX_DELAY);
}

void setup() {
    Serial.begin(9600);

    while (!Serial) { // needed to keep leonardo/micro from starting too fast!
        delay(10);
    }

    // Default address is 0x5A, if tied to 3.3V its 0x5B
    // If tied to SDA its 0x5C and if SCL then 0x5D
    if (!cap.begin(0x5A)) {
        Serial.println("MPR121 not found, check wiring?");
        while (1);
    }
    Serial.println("MPR121 found!");

    pinMode(ledPin, OUTPUT);

//  empty_keyboard_report.reportId = 0x02;
    //consumer_Report.reportId = 0x01;

    Serial.println("Starting BLE work!");
    xTaskCreate(taskServer, "server", 20000, NULL, 5, NULL);
}

void loop() {
    static bool volDirUp = true;
    // put your main code here, to run repeatedly:

    // Get the currently touched pads
    currtouched = cap.touched();

    inputKeyboard_t a{};
    for (uint8_t i=0; i<12; i++) {
        // it if *is* touched and *wasnt* touched before, alert!
        if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
            Serial.print(i); Serial.println(" touched");
            if (connected) {
//                digitalWrite(ledPin, HIGH);
//            a.Key = random(0x02,0x27);
                a.Key[i%6] = 0x02 + i;
                //   a.reportId = 0x02;
                input->setValue((uint8_t*)&a,sizeof(a));
                input->notify();
            }
        }
        // if it *was* touched and now *isnt*, alert!
        if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
            Serial.print(i); Serial.println(" released");

            if(connected) {

//                digitalWrite(ledPin, HIGH);
                a.Key[i%6] = 0x0;
//                   a.reportId = 0x02;
                input->setValue((uint8_t*)&a,sizeof(a));
                input->notify();

//                input->setValue((uint8_t*)(&empty_keyboard_report), sizeof(empty_keyboard_report));
//                input->notify();
            }
        }
    }
    // reset our state
    lasttouched = currtouched;
    delay(100);

//    digitalWrite(ledPin, LOW);
    // comment out this line for detailed data from the sensor!
//    return;
//
//    // debugging info, what
//    Serial.print("\t\t\t\t\t\t\t\t\t\t\t\t\t 0x"); Serial.println(cap.touched(), HEX);
//    Serial.print("Filt: ");
//    for (uint8_t i=0; i<12; i++) {
//        Serial.print(cap.filteredData(i)); Serial.print("\t");
//    }
//    Serial.println();
//    Serial.print("Base: ");
//    for (uint8_t i=0; i<12; i++) {
//        Serial.print(cap.baselineData(i)); Serial.print("\t");
//    }
//    Serial.println();
//
//    // put a delay so it isn't overwhelming
//    delay(100);

//    delay(500);
//    if(connected){
//        digitalWrite(ledPin, HIGH);
//        inputKeyboard_t a{};
//        a.Key = random(0x02,0x27);
//        //   a.reportId = 0x02;
//        input->setValue((uint8_t*)&a,sizeof(a));
//        input->notify();
//
//        input->setValue((uint8_t*)(&empty_keyboard_report), sizeof(empty_keyboard_report));
//        input->notify();
//
//        inputConsumer_t b{};
//   b.reportId = 0x01;

//        b.ConsumerControl = volDirUp ? 0xE9 : 0xEA;
//        volDirUp = volDirUp ? false : true;
//        inputVolume->setValue((uint8_t*)&b,sizeof(b));
//        inputVolume->notify();
//
//        inputVolume->setValue((uint8_t*)&consumer_Report,sizeof(consumer_Report));
//        inputVolume->notify();
//
//        delay(100);
//        digitalWrite(ledPin, LOW);
//    }
}

