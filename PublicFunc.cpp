#include "PublicFunc.h"


uint16_t PublicFunc::readU16LittleEndian(const uint8_t* data, int i) 
{
    return static_cast<uint16_t>(data[i]) |
        (static_cast<uint16_t>(data[++i]) << 8);
}

uint16_t PublicFunc::getValue(uint16_t v1, uint16_t v2)
{
    return v1 | (v2 << 8);
}

// CRC16Ð£Ñé¼ÆËã
uint16_t PublicFunc::calculateCRC16(const QByteArray& data)
{
    uint16_t crc = 0xFFFF;
    const uint16_t polynomial = 0xA001;

    for (char byte : data) {
        crc ^= static_cast<uint8_t>(byte);
        for (int i = 0; i < 8; i++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= polynomial;
            }
            else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

uint16_t PublicFunc::crc_16(const char* data, size_t length)
{
    uint16_t crc = 0;
    while (length--) {
        crc ^= *data++;
        for (int i = 0; i < 8; ++i) { crc = (crc & 0x0001) ? (crc >> 1) ^ 0xA001 : (crc >> 1); }
    }
    return crc;
}


int PublicFunc::getColorLevel(int value)
{
    int nMax = 700;
    int nMin = 100;
    int nStep = (nMax - nMin) / 20;
    /*  for (int i = 0; i < 20; ++i)
    {
        nMin = nMax - nStep;
        if (value <= nMax && value >nMin)
            return i;
        nMax = nMin;
    }*/


    if(700 >= value     && 670 < value)
        return 0;
    else if(670 >= value && 640 <value)
        return 1;
    else if(640 >= value && 610 < value)
        return 2;
    else if(610 >= value && 580 < value)
        return 3;
    else if(580 >= value && 550 < value)
        return 4;
    else if(550 >= value && 520 < value)
        return 5;
    else if(520 >= value && 490 < value)
        return 6;
    else if(490 >= value && 460 < value)
        return 7;
    else if(460 >= value && 430 < value)
        return 8;
    else if (430 >= value && 400 < value)
        return 9;
    else if (400 >= value && 370 < value)
        return 10;
    else if (370 >= value && 340 < value)
        return 11;
    else if (340 >= value && 310 < value)
        return 12;
    else if (310 >= value && 280 < value)
        return 12;
    else if (280 >= value && 250 < value)
        return 14;
    else if (250 >= value && 220 < value)
        return 15;
    else if (220 >= value && 190 < value)
        return 16;
    else if (190 >= value && 160 < value)
        return 17;
    else if (160 >= value && 130 < value)
        return 18;
    else /*if (130 >= value && 100 < value)*/
        return 19;

}

