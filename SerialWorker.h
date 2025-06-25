#pragma once

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <QMap>
#include "PublicFunc.h"

class SerialWorker : public QObject
{
    Q_OBJECT
public:
    explicit SerialWorker(QObject* parent = nullptr);
    ~SerialWorker();

    void setPortName(const QString& portName);
    void setBaudRate(int baudRate);
    void parsePacketData(const QByteArray& strData);

public slots:
    void openPort();
    void closePort();
    void writeData(const QByteArray& data);

private slots:
    void readData();

signals:
    void portOpened(bool success);
    void portClosed();
    void dataReceived(const QByteArray& data);
    void errorOccurred(const QString& errorString);
    void sigUpdateData(QMap<int, int> valueMap);

private:
    QSerialPort* m_serialPort;
    QString m_portName;
    int m_baudRate;
    QByteArray m_buffer;
    QMap<int, int> m_valueMap;

    void processBuffer();
};

