#pragma once

#include <stdint.h>
#include <QByteArray>


class PublicFunc
{
public:
	static uint16_t readU16LittleEndian(const uint8_t* data, int i);
	static uint16_t getValue(uint16_t v1, uint16_t v2);
	static uint16_t calculateCRC16(const QByteArray& data);
    static uint16_t crc_16(const char* data, size_t length);
};



