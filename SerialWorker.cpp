#include "SerialWorker.h"
#include <QDebug>

const int PACKET_SIZE = 247; // 假设每个数据包字节
const int CHECK_SIZE = 237;

SerialWorker::SerialWorker(QObject* parent)
    : QObject(parent), m_serialPort(nullptr), m_baudRate(9600)
{
}

SerialWorker::~SerialWorker()
{
    closePort();
    delete m_serialPort;
}

void SerialWorker::setPortName(const QString& portName)
{
    m_portName = portName;
}

void SerialWorker::setBaudRate(int baudRate)
{
    m_baudRate = baudRate;
}

void SerialWorker::openPort()
{
    if (!m_serialPort) {
        m_serialPort = new QSerialPort(this);
        m_serialPort->setPortName(m_portName);
        m_serialPort->setBaudRate(QSerialPort::Baud115200, QSerialPort::AllDirections);
        m_serialPort->setDataBits(QSerialPort::Data8);
        m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
        m_serialPort->setParity(QSerialPort::NoParity);
        m_serialPort->setStopBits(QSerialPort::OneStop);
        connect(m_serialPort, &QSerialPort::readyRead, this, &SerialWorker::readData);
        connect(m_serialPort, &QSerialPort::errorOccurred, [this](QSerialPort::SerialPortError error) {
            if (error != QSerialPort::NoError) {
                emit errorOccurred(m_serialPort->errorString());
            }
            });
    }
    if (m_serialPort->isOpen())
    {
        m_serialPort->clear();
        m_serialPort->close();
    }

    if (m_serialPort->open(QIODevice::ReadWrite)) {
        emit portOpened(true);
    }
    else {
        emit errorOccurred(m_serialPort->errorString());
        emit portOpened(false);
    }
}

void SerialWorker::closePort()
{
    if (m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->close();
        emit portClosed();
    }
}

void SerialWorker::writeData(const QByteArray& data)
{
    if (m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->write(data);
    }
    else {
        emit errorOccurred("Port is not open");
    }
}

void SerialWorker::readData()
{
    if (!m_serialPort || !m_serialPort->isOpen()) {
        return;
    }
    QByteArray strData = m_serialPort->readAll();
    qDebug() << "data:" << strData.toHex();
    m_buffer.append(strData);
    processBuffer();
}

void SerialWorker::processBuffer()
{
    while (m_buffer.size() >= PACKET_SIZE) {
        uint8_t address = static_cast<uint8_t>(m_buffer[0]);
        if (m_buffer.at(0) != 0x3C || m_buffer.at(1) != 0x3C) {
            m_buffer.remove(0, 1);
            continue;
        }

        // 检查功能码是否有效
        uint8_t functionCode = static_cast<uint8_t>(m_buffer[1]);
        if (functionCode > 0x2B) {
            m_buffer.remove(0, 2);
            continue;
        }

        QByteArray packet = m_buffer.left(PACKET_SIZE);
        m_buffer.remove(0, PACKET_SIZE);

        if (packet.at(0) == 0x3C && packet.at(1) == 0x3C && packet.at(PACKET_SIZE - 1) == 0x3C && packet.at(PACKET_SIZE - 2)) {
            QByteArray sumPacket = packet.right(CHECK_SIZE);
            uint16_t checksum = PublicFunc::crc_16(reinterpret_cast<const  uint8_t*>(sumPacket.constData()), CHECK_SIZE);
            char realSum = PublicFunc::getValue(packet.at(PACKET_SIZE - -4), packet.at(PACKET_SIZE - -3));
            if (checksum == realSum) { // 校验和验证
                parsePacketData(packet);
                sigUpdateData(m_valueMap);
            }

        }
    }
}

void SerialWorker::parsePacketData(const QByteArray& strData)
{
    int index = 9;
    for (int i = 0; i < 9; ++i)
    {
        if (2 == i)
        {
            int value = PublicFunc::getValue(strData[i * 13 * 2 + 3 * 2 + index], strData[i * 13 * 2 + 3 * 2 + 1 + index]);
            value = value < 100 ? 100 : value;
            value = value > 700 ? 700 : value;
            m_valueMap.insert(i, value);
            m_valueMap.insert(1, value);
        }
        else if (3 == i)
        {
            int value = PublicFunc::getValue(strData[i * 13 * 2 + 0 * 2 + index], strData[i * 13 * 2 + 0 * 2 + 1 + index]);
            value = value < 100 ? 100 : value;
            value = value > 700 ? 700 : value;
            m_valueMap.insert(i, value);
        }
        else if (5 == i || 6 == i)
        {
            int value = PublicFunc::getValue(strData[i * 13 * 2 + 2 * 2 + index], strData[i * 13 * 2 + 2 * 2 + 1 + index]);
            value = value < 100 ? 100 : value;
            value = value > 700 ? 700 : value;
            m_valueMap.insert(i, value);
        }
        else if (7 == i)
        {
            int value = PublicFunc::getValue(strData[i * 13 * 2 + 8 * 2 + index], strData[i * 13 * 2 + 8 * 2 + 1 + index]);
            value = value < 100 ? 100 : value;
            value = value > 700 ? 700 : value;
            m_valueMap.insert(i, value);
        }
        else if (1 == i || 4 == i)
            continue;
        else
        {
            int value = PublicFunc::getValue(strData[i * 13 * 2 + 5 * 2 + index], strData[i * 13 * 2 + 5 * 2 + 1 + index]);
            value = value < 100 ? 100 : value;
            value = value > 700 ? 700 : value;
            m_valueMap.insert(1, value);
        }
        /* int sum = 0;
         for(int j= 0; j<12; j+=2)
         {
             sum += getValue(strData[i*13 + j], strData[i*13 + j+1]);
         }
         m_valueMap.insert(i, sum / 12);
         if(2 == i)
             m_valueMap.insert(1, sum / 12);*/
    }
}

