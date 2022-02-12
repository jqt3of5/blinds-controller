//
// Created by jqt3o on 2/9/2022.
//
#include "blinds.h"
#include <Arduino.h>



Blinds:: Blinds(int pin)
{
    _pin = pin;
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void inline Blinds::writeBlock(int32_t bits, unsigned int count)
{
    for (int i = count-1; i >= 0; i--)
    {
        bool v = bits & (1 << i);

        digitalWrite(_pin, HIGH);
        delayMicroseconds(v ? 688 : 305);
        digitalWrite(_pin, LOW);
        delayMicroseconds(v ? 305 : 688);
    }
}

void Blinds::sendCommand(BlindsCommand command, BlindsChannel channel) {

    for (int i = 0; i < 6; ++i)
    {
        //start pattern
        digitalWrite(_pin, HIGH);
        //Subsequent calls need a longer start
        delayMicroseconds(i == 0 ? 4000 : 5000);
        digitalWrite(_pin, LOW);
        delayMicroseconds(2400);
        digitalWrite(_pin, HIGH);
        delayMicroseconds(950);

        writeCommand(command, channel);

    }

}

void Blinds::writeCommand(BlindsCommand command, BlindsChannel channel)
{
    int preamble = 0b1101110100110110;

    writeBlock(preamble, 16);
    writeBlock(1, 1);

    writeBlock(command, 12);
    writeBlock(channel, 4);

    switch(command)
    {
        case BlindsCommandUp:
            writeBlock(BlindsCommandCRCUp, 4);
            break;
        case BlindsCommandDown:
            writeBlock(BlindsCommandCRCDown, 4);
            break;
        case BlindsCommandOpen:
            writeBlock(BlindsCommandCRCOpen, 4);
            break;
        case BlindsCommandClose:
            writeBlock(BlindsCommandCRCClose, 4);
            break;
        case BlindsCommandStop:
            writeBlock(BlindsCommandCRCStop, 4);
            break;
    }

    switch(channel)
    {
        case BlindsChannel1:
            writeBlock(BlindsChannelCRC1, 4);
            break;
        case BlindsChannel2:
            writeBlock(BlindsChannelCRC2, 4);
            break;
        case BlindsChannel3:
            writeBlock(BlindsChannelCRC3, 4);
            break;
        case BlindsChannel4:
            writeBlock(BlindsChannelCRC4, 4);
            break;
        case BlindsChannel5:
            writeBlock(BlindsChannelCRC5, 4);
            break;
        case BlindsChannel6:
            writeBlock(BlindsChannelCRC6, 4);
            break;
    }
    writeBlock(1, 1);
}
