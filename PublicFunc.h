#pragma once

#include <stdint.h>
#include <QByteArray>
#include <QMap>


struct ST_ViewParam
{
	int nReadTime = 300;
	int nDelayTime = 30;
	QMap<int, bool> showValueMap;
	QMap<int, int> indexMap;
};

class PublicFunc
{
public:
	static uint16_t readU16LittleEndian(const uint8_t* data, int i);
	static uint16_t getValue(uint16_t v1, uint16_t v2);
	static uint16_t calculateCRC16(const QByteArray& data);
    static uint16_t crc_16(const char* data, size_t length);
    static int getColorLevel(int value);
};



