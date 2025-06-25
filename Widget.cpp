#include "Widget.h"
#include "ui_Widget.h"
#include <QApplication>
#include <QPainter>
#include <QScreen>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QDebug>
#include <windows.h>
#include <wingdi.h>
#include <QRandomGenerator>
#include <QVariantAnimation>
#include <QMessageBox>
#include "SerialWorker.h"
#include "PublicFunc.h"

const int PACKET_DATA_SIZE = 247; // 假设每个数据包字节
const int CHECK_DATA_SIZE = 237;



Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget),
    m_serialThread(new QThread(this)),
    m_serialWorker(new SerialWorker)
{
    ui->setupUi(this);

    initUi();
    initColorData();
    ui->cmbBox_port->clear();
    ui->cmbBox_port->addItems(getSerialPortList());

    connect(m_serialWorker, &SerialWorker::sigUpdateData, this, &Widget::onUpdateData);
    connect(m_serialWorker, &SerialWorker::portOpened, this, &Widget::onPortOpened);
    connect(this, &Widget::sigOpenPort, m_serialWorker, &SerialWorker::openPort);
    connect(this, &Widget::sigClosePort, m_serialWorker, &SerialWorker::closePort);
    connect(m_serialThread, &QThread::finished, m_serialWorker, &SerialWorker::deleteLater);
    connect(m_serialThread, &QThread::finished, m_serialThread, &QThread::deleteLater);
    m_serialWorker->moveToThread(m_serialThread);

    m_pTimer = new QTimer(this);
    connect(m_pTimer, &QTimer::timeout, this, &Widget::onTimeout);

    m_pAnim = new QVariantAnimation(this);
    m_pAnim->setDuration(100);
  //  connect(m_pAnim, &QVariantAnimation::valueChanged, this, &Widget::onAnimationValueChanged);

    QTimer::singleShot(10000,this,[this](){
        initData();
        this->update();
        m_pTimer->start(100);
    });

    m_serialThread->start();
}

void Widget::initAnimMap()
{
    QMap<int, QRect>::iterator it = m_areaMap.begin();
    for (it; it != m_areaMap.end(); ++it)
    {
        QVariantAnimation* pAnim = new QVariantAnimation(this);
        pAnim->setDuration(100);
        connect(pAnim, &QVariantAnimation::valueChanged, this, &Widget::onAnimationValueChanged);
        m_animMap.insert(pAnim, it.key());
    }
}

Widget::~Widget()
{
    m_serialThread->quit();
    m_serialThread->wait();
    delete ui;
    //m_file.close();
}

void Widget::initUi()
{
    this->setWindowIcon(QIcon(":/images/images/pressure.png"));
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_StaticContents);
    //setStyleSheet("background: #00FFFF;"); // 如果需要样式表

    //设置无边框
    this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);
    //实例阴影shadow

    //实例阴影shadow
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    //设置阴影距离
    shadow->setOffset(0, 0);
    //设置阴影颜色
    shadow->setColor(QColor(0, 0, 0, 76));
    //设置阴影圆角
    shadow->setBlurRadius(10);
    //给嵌套QWidget设置阴影
    this->setGraphicsEffect(shadow);
}

void Widget::initPort()
{
    QString strSerialPort = ui->cmbBox_port->currentText();
    if (m_serialPort.isOpen())
    {
        m_serialPort.clear();
        m_serialPort.close();
    }

    m_serialPort.setPortName(strSerialPort);
    m_serialPort.setBaudRate(QSerialPort::Baud115200, QSerialPort::AllDirections);
    m_serialPort.setDataBits(QSerialPort::Data8);
    m_serialPort.setFlowControl(QSerialPort::NoFlowControl);
    m_serialPort.setParity(QSerialPort::NoParity);
    m_serialPort.setStopBits(QSerialPort::OneStop);
    connect(&m_serialPort, &QSerialPort::readyRead, this, &Widget::onReciveData);
    if (!m_serialPort.open(QIODevice::ReadOnly))
    {
        QMessageBox::information(this, tr("Tips"), tr("Failed to open the serialport"));
        return;
    }
    ui->btnOpen->setEnabled(false);
}

QStringList Widget::getSerialPortList()
{
    QStringList serialPortList;
    for(const auto& port : QSerialPortInfo::availablePorts())
    {
        serialPortList.push_back(port.portName());
    }

    return serialPortList;
}

void Widget::onReciveData()
{
    QByteArray strData = m_serialPort.readAll();
    qDebug() << "data:" << strData.toHex();

    processFixedLengthData(strData);
}

void Widget::processFixedLengthData(const QByteArray& data)
{
    static QByteArray buffer;

    buffer.append(data);

    while (buffer.size() >= PACKET_DATA_SIZE) {
        uint8_t address = static_cast<uint8_t>(buffer[0]);
        if (buffer.at(0) != 0x3C || buffer.at(1) != 0x3C) {
            buffer.remove(0, 1);
            continue;
        }

        // 检查功能码是否有效
        uint8_t functionCode = static_cast<uint8_t>(buffer[1]);
        if (functionCode > 0x2B) { 
            buffer.remove(0, 2);
            continue;
        }

        QByteArray packet = buffer.left(PACKET_DATA_SIZE);
        buffer.remove(0, PACKET_DATA_SIZE);

        if (packet.at(0) == 0x3C && packet.at(1) == 0x3C && packet.at(PACKET_DATA_SIZE - 1) == 0x3C && packet.at(PACKET_DATA_SIZE - 2)) { 
            QByteArray sumPacket = packet.right(CHECK_DATA_SIZE);
            uint16_t checksum = PublicFunc::crc_16(reinterpret_cast<const  uint8_t*>(sumPacket.constData()), CHECK_DATA_SIZE);
            char realSum = PublicFunc::getValue(packet.at(PACKET_DATA_SIZE - -4), packet.at(PACKET_DATA_SIZE - -3));
            if (checksum == realSum) { // 校验和验证
                parsePacketData(packet);

                update();
            }

        }
    }
}

void Widget::parsePacketData(const QByteArray& strData)
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

void Widget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

     QPainter painter(this);
     
     painter.setRenderHint(QPainter::Antialiasing, true);
   //  painter.eraseRect(rect());
    painter.fillRect(this->rect(), Qt::gray);
    painter.fillRect(ui->widget->geometry(), Qt::white);
    painter.drawImage(ui->label->geometry(), QImage(":/images/images/chair.png"));

    for(QMap<int, QRect>::iterator it = m_areaMap.begin(); it != m_areaMap.end(); ++it)
    {
        if (it != m_areaMap.begin())
            continue;

        QRect rect = it.value();
        int key = it.key();
        int radio = rect.width() > rect.height() ? rect.width() / 2 : rect.height() / 2;
        QRadialGradient radialGrad(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2,
                                    rect.width() > rect.height() ? rect.width() / 2 : rect.height() / 2);

        int value = m_valueMap.value(key);
        int level = getColorLevel(value);
     //   qDebug() << "value:" << value << "level:"<<level<< "  ########################";
        float step = 1.0 / (m_colorMap.size() - level);
        int colorCount = (m_colorMap.size() - level) > 4 ? 4 : (m_colorMap.size() - level);
        int wStep = (rect.width() - rect.width() * 0.3) / colorCount;
        int hStep = (rect.height() - rect.height() * 0.3) / colorCount;
        radialGrad.setColorAt(0.0, m_colorMap.value(level));
        int j = 0;
        int colorIndex = level;
        bool bInit = true;
        if (!bInit)
        {
            bInit = false;
            continue;
        }
        for(int i=level; i < m_colorMap.size(); i++)
        {
            QColor clr = m_colorMap.value(colorIndex + colorCount -1 - j);
           // clr = QColor(130, 170, 229, 255);
            radialGrad.setColorAt(1 - j*step, m_colorMap.value(colorIndex + colorCount -1 - j));
            QPoint pt = QPoint(rect.x() + j * wStep, rect.y() + j * hStep);
            QRect tmprect = QRect(rect.x() + j * wStep / 2, rect.y() + j * hStep / 2, rect.width() - (j * wStep), rect.height() - (j * hStep));
         
            painter.setBrush(radialGrad); // 设置画笔的填充颜色为渐变
            painter.setPen(Qt::NoPen); // 不绘制边框
            painter.drawEllipse(tmprect); // 绘制椭圆，填充渐变色
         //   qDebug()<<"key:"<<key << " tmpRect:" << tmprect << "  colorIndex:" << colorIndex <<"  colorCount:" << colorCount << "   color:" << clr;
            ++colorIndex;
            ++j;
            if (j >= colorCount)
                break;
        }

    }
  //  qDebug() << "update ----------------------------------------------------------------------------:";

}

void Widget::resizeEvent(QResizeEvent *e)
{
  
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void Widget::initData()
{
    QRect rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.49) ,
                       (ui->label->geometry().y() +  ui->label->height() * 0.25), ui->label->width() * 0.107, ui->label->height() * 0.185);
    m_areaMap.insert(0, rect);

    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.49) ,
                 ui->label->height() * 0.5, ui->label->width() * 0.107, ui->label->height() * 0.128);
    m_areaMap.insert(3, rect);

    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.37) ,
                 (ui->label->geometry().y() +  ui->label->height() * 0.37), ui->label->width() * 0.053, ui->label->height() * 0.164);
    m_areaMap.insert(5, rect);

    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.67) ,
                 (ui->label->geometry().y() +  ui->label->height() * 0.37), ui->label->width() * 0.053, ui->label->height() * 0.164);
    m_areaMap.insert(6, rect);

    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.41) ,
                 (ui->label->geometry().y() +  ui->label->height() * 0.6), ui->label->width() * 0.107, ui->label->height() * 0.107);
    m_areaMap.insert(7, rect);

    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.57) ,
                 (ui->label->geometry().y() +  ui->label->height() * 0.6), ui->label->width() * 0.107, ui->label->height() * 0.107);
    m_areaMap.insert(8, rect);

    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.37) ,
                 (ui->label->geometry().y() +  ui->label->height() * 0.72), ui->label->width() * 0.107, ui->label->height() * 0.1);
    m_areaMap.insert(1, rect);

    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.60) ,
                 (ui->label->geometry().y() +  ui->label->height() * 0.72), ui->label->width() * 0.107, ui->label->height() * 0.1);
    m_areaMap.insert(2, rect);

    for(int i=0; i<9; ++i)
    {
        if(1 == i || 4 == i)
            continue;
        int sum = 0;
        int randomValue = QRandomGenerator::global()->bounded(5, 11)*100;
        randomValue = 700;
        m_valueMap.insert(i, randomValue);
        if(2 == i)
            m_valueMap.insert(1, randomValue);
    }

    initAnimMap();
}

void Widget::initColorData()
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

int Widget::getColorLevel(int value)
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

void Widget::on_btnOpen_clicked()
{
  //  initPort();

    m_serialWorker->setPortName(ui->cmbBox_port->currentText());
    Q_EMIT sigOpenPort();
}


void Widget::on_btnClose_clicked()
{
    m_serialPort.close();
    this->close();
}

void Widget::onTimeout()
{
    for (QMap<int, int>::iterator it = m_valueMap.begin(); it != m_valueMap.end(); ++it)
    {
        
        int value = it.value();
        QColor startColor = m_colorMap.value(getColorLevel(value));
        value -= 30;
        int key = it.key();
        m_valueMap[key] = value;
        if (value <= 100)
            m_pTimer->stop();
        
        QColor endColor = m_colorMap.value(getColorLevel(value));
        update();

        /*for (int aKey : m_animMap)
        {
            if (aKey == key)
            {
                m_animMap.key(aKey)->setStartValue(startColor);   
                m_animMap.key(aKey)->setEndValue(startColor);
                m_animMap.key(aKey)->start();
                break;
            }
        }*/
    }
}

void Widget::onAnimationValueChanged(const QVariant& value)
{
    QVariantAnimation* pAnim = dynamic_cast<QVariantAnimation*>(sender());
    if (!pAnim)
        return;

    QColor c = value.value<QColor>();
    int key = m_animMap.value(pAnim);
    m_colorMap[key] = c;
    update();
}

void Widget::onUpdateData(QMap<int, int> valueMap)
{
    m_valueMap = valueMap;
    this->update();
}

void Widget::onPortOpened(bool success)
{
    if (!success)
    {
        QMessageBox::information(this, tr("Tips"), tr("Failed to open the serialport"));
    }
}

