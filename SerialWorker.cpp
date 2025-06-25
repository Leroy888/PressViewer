#include "SerialWorker.h"
#include <QDebug>

const int PACKET_SIZE = 247; // 假设每个数据包字节
const int CHECK_SIZE = 237;
const int READ_TIME = 300;
const float UPDATE_NUM = 10.0;

SerialWorker::SerialWorker(QObject* parent)
    : QObject(parent), m_serialPort(nullptr), m_baudRate(9600),
    m_bPuase(false)
{
    m_pTimer = new QTimer(this);
    connect(m_pTimer,&QTimer::timeout,this,&SerialWorker::onTimeout);

    m_pUpdateTimer = new QTimer(this);
    connect(m_pUpdateTimer,&QTimer::timeout,this,&SerialWorker::onUpdateTimeout);
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

    m_pTimer->start(READ_TIME);
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
    m_buffer.append(strData);
    processBuffer();
}

void SerialWorker::processBuffer()
{
    while (m_buffer.size() >= PACKET_SIZE && !m_bPuase) {

        uint8_t address = static_cast<uint8_t>(m_buffer[0]);
        if (m_buffer.at(0) != 0x3C || m_buffer.at(1) != 0x3C) {
            m_buffer.remove(0, 1);
            continue;
        }

        QByteArray packet = m_buffer.left(PACKET_SIZE);
        m_buffer.remove(0, PACKET_SIZE);
       // qDebug()<<"packet size:"<<packet.size()<< " packet:"<<packet.toHex();
        if (packet.at(0) == 0x3C && packet.at(1) == 0x3C) /*&& packet.at(PACKET_SIZE - 1) == 0x3E && 0x3E == packet.at(PACKET_SIZE - 2))*/ {
            QByteArray sumPacket = packet.right(CHECK_SIZE);
           // char checksum = PublicFunc::crc_16(reinterpret_cast<const  char*>(sumPacket.constData()), CHECK_SIZE);
           // char realSum = PublicFunc::getValue(packet.at(PACKET_SIZE - -4), packet.at(PACKET_SIZE - -3));
           // qDebug()<<"checkSum:"<<checksum<<"  realSum:"<<realSum;
           // qDebug()<<"--------------------";
           // if (checksum == realSum)
            { // 校验和验证
                parsePacketData(packet);
                //sigUpdateData(m_valueMap);
                if(m_lastValueMap.isEmpty())
                    m_lastValueMap = m_valueMap;
                for(int key : m_valueMap.keys())
                {
                    int step = (m_valueMap.value(key) - m_lastValueMap.value(key)) / UPDATE_NUM;
                    m_stepMap.insert(key, step);
                }
                m_pUpdateTimer->start(300 / UPDATE_NUM);
                m_buffer.clear();
                m_bPuase = true;
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
            int value = PublicFunc::getValue(strData[i * 13 * 2 + 4 * 2 + index], strData[i * 13 * 2 + 4 * 2 + 1 + index]);
           // qDebug()<<"1 row value:"<<value;
            value = value < 100 ? 100 : value;
            value = value > 700 ? 700 : value;
            m_valueMap.insert(i, value);
            m_valueMap.insert(1, value);
        }
        else if (3 == i)
        {
            int value = PublicFunc::getValue(strData[i * 13 * 2 + 0 * 2 + index], strData[i * 13 * 2 + 0 * 2 + 1 + index]);
           // qDebug()<<"3 row value:"<<value;
            value = value < 100 ? 100 : value;
            value = value > 700 ? 700 : value;
            m_valueMap.insert(i, value);
        }
        else if (5 == i)
        {
            QByteArray tmpArry = strData.mid(i * 13 * 2+ index, i * 13 * 3+ index);
           // qDebug()<<i<<" tmpArry:"<<tmpArry.toHex();
            int value = PublicFunc::getValue(strData[i * 13 * 2 + 0 * 2 + index], strData[i * 13 * 2 + 0 * 2 + 1 + index]);
           // qDebug()<<i<<" row value:"<<value;
            value = value < 100 ? 100 : value;
            value = value > 700 ? 700 : value;
            m_valueMap.insert(i, value);
        }
        else if ( 6 == i)
        {
            QByteArray tmpArry = strData.mid(i * 13 * 2+ index, i * 13 * 2 + 26 + index);
            qDebug()<<i<<" tmpArry:"<<tmpArry;
            int value =0;
            for(int j = 0; j<12 ; ++j){
                qDebug()<<i<<" tmpArry: value1:"<<strData[i * 13 * 2 + j * 2 + index]<<"  value2:"<<strData[i * 13 * 2 + j * 2 + 1 + index];
               value = PublicFunc::getValue(strData[i * 13 * 2 + j * 2 + index], strData[i * 13 * 2 + j * 2 + 1 + index]);
                qDebug()<<"---------value:"<<value;
                if(value > 0)
                   break;
            }
           qDebug()<<i<<" row value:"<<value;
            value = value < 100 ? 100 : value;
            value = value > 700 ? 700 : value;
            m_valueMap.insert(i, value);
        }
        else if (7 == i | 8 == i)
        {
            int value = PublicFunc::getValue(strData[i * 13 * 2 + 3 * 2 + index], strData[i * 13 * 2 + 3 * 2 + 1 + index]);
         //   qDebug()<<i<<" row value:"<<value;
            value = value < 100 ? 100 : value;
            value = value > 700 ? 700 : value;
            m_valueMap.insert(i, value);
        }
        else if (1 == i || 4 == i)
            continue;
        else
        {
            int value = PublicFunc::getValue(strData[i * 13 * 2 + 5 * 2 + index], strData[i * 13 * 2 + 5 * 2 + 1 + index]);
         //   qDebug()<<i<<" row value:"<<value;
            value = value < 100 ? 100 : value;
            value = value > 700 ? 700 : value;
            m_valueMap.insert(i, value);
        }
    }
}

void SerialWorker::onTimeout()
{
    m_lastValueMap = m_valueMap;
    m_pUpdateTimer->stop();
    m_bPuase = false;
}

void SerialWorker::onUpdateTimeout()
{
    static int num = 0;
    for(int key : m_valueMap.keys())
    {
        int step = m_stepMap.value(key);
        m_valueMap.insert(key, m_valueMap.value(key) - step);
    }
    emit sigUpdateData(m_valueMap);
    ++num;
    if(num == UPDATE_NUM){
        m_pUpdateTimer->stop();
        num = 0;
    }
}

