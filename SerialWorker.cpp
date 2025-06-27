#include "SerialWorker.h"
#include <QDebug>

const int PACKET_SIZE = 283; // 假设每个数据包字节
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

    initColorData();
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

void SerialWorker::SetParams(const ST_ViewParam& stParam)
{
    m_stParam = stParam;
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

    m_pTimer->start(m_stParam.nReadTime);
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
        if (packet.at(0) == 0x3C && packet.at(1) == 0x3C ) /*&& packet.at(PACKET_SIZE - 1) == 0x3E && 0x3E == packet.at(PACKET_SIZE - 2))*/ {
            QByteArray sumPacket = packet.right(CHECK_SIZE);
           // char checksum = PublicFunc::crc_16(reinterpret_cast<const  char*>(sumPacket.constData()), CHECK_SIZE);
           // char realSum = PublicFunc::getValue(packet.at(PACKET_SIZE - -4), packet.at(PACKET_SIZE - -3));
           // qDebug()<<"checkSum:"<<checksum<<"  realSum:"<<realSum;

            { // 校验和验证
                parsePacketData(packet);
                //sigUpdateData(m_valueMap);
                if(m_lastValueMap.isEmpty())
                    m_lastValueMap = m_valueMap;
                for(int key : m_valueMap.keys())
                {
                    int step = (m_valueMap.value(key) - m_lastValueMap.value(key)) / m_stParam.nDelayTime;
                    m_stepMap.insert(key, step);
                }

                m_pUpdateTimer->start(m_stParam.nDelayTime);
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
        int nColmn = m_stParam.indexMap.value(i);
        if (2 == i)
        {
            int value = PublicFunc::getValue(strData[i * 13 * 2 + nColmn * 2 + index], strData[i * 13 * 2 + nColmn * 2 + 1 + index]);
           // qDebug()<<"1 row value:"<<value;
            value = value < 100 ? 100 : value;
            value = value > 700 ? 700 : value;
            m_valueMap.insert(i, value);
            m_valueMap.insert(1, value);
        }
        else if (3 == i)
        {
            int value = PublicFunc::getValue(strData[i * 13 * 2 + nColmn * 2 + index], strData[i * 13 * 2 + nColmn * 2 + 1 + index]);
           // qDebug()<<"3 row value:"<<value;
            value = value < 100 ? 100 : value;
            value = value > 700 ? 700 : value;
            m_valueMap.insert(i, value);
        }
        else if (5 == i)
        {
            QByteArray tmpArry = strData.mid(i * 13 * 2+ index, i * 13 * 3+ index);
           // qDebug()<<i<<" tmpArry:"<<tmpArry.toHex();
            int value = PublicFunc::getValue(strData[i * 13 * 2 + nColmn * 2 + index], strData[i * 13 * 2 + nColmn * 2 + 1 + index]);
           // qDebug()<<i<<" row value:"<<value;
            value = value < 100 ? 100 : value;
            value = value > 700 ? 700 : value;
            m_valueMap.insert(i, value);
        }
        else if ( 4 == i)
        {
            QByteArray tmpArry = strData.mid(i * 13 * 2+ index, i * 13 * 2 + 26 + index);
            qDebug()<<i<<" tmpArry:"<<tmpArry;
            int value =0;
            // for(int j = 12; j<13 ; ++j){
            //     qDebug()<<i<<" tmpArry: value1:"<<strData[i * 13 * 2 + j * 2 + index]<<"  value2:"<<strData[i * 13 * 2 + j * 2 + 1 + index];
            //    value = PublicFunc::getValue(strData[i * 13 * 2 + j * 2 + index], strData[i * 13 * 2 + j * 2 + 1 + index]);
            //     qDebug()<<"---------value:"<<value;
            //     if(value > 0)
            //        break;
            // }
            value = PublicFunc::getValue(strData[i * 13 * 2 + nColmn * 2 + index], strData[i * 13 * 2 + nColmn * 2 + 1 + index]);

           qDebug()<<i<<" row value:"<<value;
            value = value < 100 ? 100 : value;
            value = value > 700 ? 700 : value;
            m_valueMap.insert(6, value);
        }
        else if (7 == i | 8 == i)
        {
            int value = PublicFunc::getValue(strData[i * 13 * 2 + nColmn * 2 + index], strData[i * 13 * 2 + nColmn * 2 + 1 + index]);
         //   qDebug()<<i<<" row value:"<<value;
            value = value < 100 ? 100 : value;
            value = value > 700 ? 700 : value;
            m_valueMap.insert(i, value);
        }
        else if (1 == i || 6 == i)
            continue;
        else
        {
            int value = PublicFunc::getValue(strData[i * 13 * 2 + nColmn * 2 + index], strData[i * 13 * 2 + nColmn * 2 + 1 + index]);
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
    if(num == m_stParam.nReadTime / m_stParam.nDelayTime){
        m_pUpdateTimer->stop();
        num = 0;
    }
}

void SerialWorker::initColorData()
{
    int a = 255;
    int step = 10;
    m_colorMap.insert(0, QColor(222, 74, 63, a));
    m_colorMap.insert(1, QColor(222, 100, 63, a-=step));
    m_colorMap.insert(2, QColor(222, 123, 63, a -= step));
    m_colorMap.insert(3, QColor(222, 145, 63, a -= step));

    m_colorMap.insert(4, QColor(222, 171, 63, a -= step));
    m_colorMap.insert(5, QColor(222, 190, 63, a -= step));
    m_colorMap.insert(6, QColor(237, 231, 69, a -= step));
    m_colorMap.insert(7, QColor(237, 199, 69, a -= step));
    m_colorMap.insert(8, QColor(215, 204, 82, a -= step));

    m_colorMap.insert(9, QColor(207, 215, 82, a -= step));
    m_colorMap.insert(10, QColor(193, 197, 80, a -= step));
    m_colorMap.insert(11, QColor(160, 197, 80, a -= step));
    m_colorMap.insert(12, QColor(119, 197, 80, a -= step));
    m_colorMap.insert(13, QColor(80, 197, 116, a -= step));
    m_colorMap.insert(14, QColor(80, 197, 141, a -= step));

    m_colorMap.insert(15, QColor(80, 197, 185, a -= step));
    m_colorMap.insert(16, QColor(80, 179, 197, a -= step));
    m_colorMap.insert(17, QColor(77, 168, 211, a -= step));
    m_colorMap.insert(18, QColor(77, 143, 211, a -= step));
    m_colorMap.insert(19, QColor(130, 170, 229, a -= step));

}

void SerialWorker::updateShowColor()
{
    for(int key : m_valueMap.keys())
    {
        int value = m_valueMap.value(key);
        int level = PublicFunc::getColorLevel(value);
        int colorCount = (m_colorMap.size() - level) > 4 ? 4 : (m_colorMap.size() - level);
        int colorIndex = level;
        QList<QColor> clrList;
        for(int i=level; i < m_colorMap.size(); i++)
        {
            QColor clr = m_colorMap.value(colorIndex + colorCount -1 - i);
            clrList.append(clr);
        }
        for(int i=clrList.length(); i<colorCount; i++)
        {
            clrList.append(m_colorMap.value(100));
        }
        m_showColorMap.insert(key, clrList);
    }

    for(int key : m_valueMap.keys())
    {
        int value = m_valueMap.value(key);
        int level = PublicFunc::getColorLevel(value);
        int colorCount = (m_colorMap.size() - level) > 4 ? 4 : (m_colorMap.size() - level);
        int colorIndex = level;
        QList<QColor> clrList;
        for(int i=level; i < m_colorMap.size(); i++)
        {
            QColor clr = m_colorMap.value(colorIndex + colorCount -1 - i);
            clrList.append(clr);
        }
        for(int i=clrList.length(); i< 4 ; i++)
        {
            clrList.append(m_colorMap.value(100));
        }
        m_lastShowColorMap.insert(key, clrList);
    }
}

void SerialWorker::updateColor()
{
    static int num = 0;
    ++num;
    float stp = 1.0 /( (float)m_stParam.nReadTime / (float)m_stParam.nDelayTime);
    for(int key : m_showColorMap.keys())
    {
        QList<QColor> clrList1 = m_lastShowColorMap.value(key);
        QList<QColor> clrList2 = m_showColorMap.value(key);
        QList<QColor> clrList;
        for(int i = 0; i< clrList1.length(); i++)
        {
            float value = num*stp;
            QColor m_color1 = clrList1.at(i);
            QColor m_color2 = clrList2.at(i);
            int r = m_color1.red()*(1 -value) + m_color2.red()*value;
            int g = m_color1.green()*(1 -value) + m_color2.green()*value;
            int b = m_color1.blue()*(1 -value) + m_color2.blue()*value;
            clrList.append(QColor(r, g, b));
        }
        m_realColorMap.insert(key, clrList);
    }
}

