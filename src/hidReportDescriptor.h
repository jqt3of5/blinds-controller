//
// Created by jqt3o on 2/10/2022.
//

#ifndef TOUCH_PAD_HIDREPORTDESCRIPTOR_H
#define TOUCH_PAD_HIDREPORTDESCRIPTOR_H


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

#endif //TOUCH_PAD_HIDREPORTDESCRIPTOR_H
