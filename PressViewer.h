#ifndef PRESSVIEWER_H
#define PRESSVIEWER_H

#include <QFrame>

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFile>
#include <QRect>
#include <QMap>
#include <QLabel>

class PressViewer : public QFrame
{
    Q_OBJECT
public:
    PressViewer();
    ~PressViewer();

protected:
    void initUi();
    void initPort();
    QStringList getSerialPortList();

    void paintEvent(QPaintEvent *event) override ;
    void resizeEvent(QResizeEvent* e) override;
    void initData();
    void initColorData();
    int getColorLevel(int value);

    uint16_t readU16LittleEndian(const uint8_t* data, int i);
    uint16_t getValue(uint16_t v1, uint16_t v2);

protected slots:
    void onReciveData();

private slots:
    void on_btnOpen_clicked();

private:
    QLabel* m_pLabel;
    QSerialPort m_serialPort;
    QSerialPortInfo m_serialPortInfo;
    QString m_strPath;
    QFile m_file;
    QMap<int, QRect> m_areaMap;
    QMap<int, int> m_valueMap;
    QMap<int,QColor> m_colorMap;
};

#endif // PRESSVIEWER_H
