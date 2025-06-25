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

uint16_t PublicFunc::crc_16(const uint8_t* data, size_t length)
{
    uint16_t crc = 0;
    while (length--) {
        crc ^= *data++;
        for (int i = 0; i < 8; ++i) { crc = (crc & 0x0001) ? (crc >> 1) ^ 0xA001 : (crc >> 1); }
    }
    return crc;
}
