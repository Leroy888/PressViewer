#pragma once

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <QMap>
#include <QTimer>
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
    void SetParams(const ST_ViewParam& stParam);

public slots:
    void openPort();
    void closePort();
    void writeData(const QByteArray& data);

private slots:
    void readData();
    void onTimeout();
    void onUpdateTimeout();
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
    QMap<int, int> m_lastValueMap;
    QTimer* m_pTimer;
    bool m_bPuase;
    QTimer* m_pUpdateTimer;
    QMap<int, int> m_stepMap;

    ST_ViewParam m_stParam;

    void processBuffer();
};

