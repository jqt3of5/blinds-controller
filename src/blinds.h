//
// Created by jqt3o on 2/9/2022.
//

#ifndef TOUCH_PAD_BLINDS_H
#define TOUCH_PAD_BLINDS_H

#include <stdint.h>
#include <ArduinoHA.h>

//Last 4 bits are crc codes
enum BlindsCommand : uint16_t {
    BlindsCommandUp = 0b001110001000,
    BlindsCommandDown = 0b100010001000,
    BlindsCommandStop = 0b101010000000,
    BlindsCommandOpen = 0b001110000000,
    BlindsCommandClose = 0b100010000000
};

enum BlindsCommandCRC : uint8_t {
    BlindsCommandCRCUp = 0b1000,
    BlindsCommandCRCDown = 0b0011,
    BlindsCommandCRCStop = 0b1001,
    BlindsCommandCRCOpen = 0b0100,
    BlindsCommandCRCClose = 0b1011
};


enum BlindsChannel : uint8_t{
    BlindsChannel1 = 0b1000,
    BlindsChannel2 = 0b0100,
    BlindsChannel3 = 0b1100,
    BlindsChannel4 = 0b0010,
    BlindsChannel5 = 0b1010,
    BlindsChannel6 = 0b0110,
};

enum BlindsChannelCRC : uint8_t{
    BlindsChannelCRC1 = 0b0011,
    BlindsChannelCRC2 = 0b1101,
    BlindsChannelCRC3 = 0b0101,
    BlindsChannelCRC4 = 0b1001,
    BlindsChannelCRC5 = 0b1001,
    BlindsChannelCRC6 = 0b0111,
};

class Blinds {
public:

    Blinds(int pin, BlindsChannel channel);
    void sendCommand(HACover::CoverCommand command);

private:
    void writeBlock(int32_t bits, unsigned int count);
    void writeCommand(BlindsCommand command, BlindsChannel channel);
    int _pin;
    BlindsChannel _channel;
};

#endif //TOUCH_PAD_BLINDS_H
