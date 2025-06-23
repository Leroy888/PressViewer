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

uint16_t readU16LittleEndian(const uint8_t* data, int i) {
    return static_cast<uint16_t>(data[i]) |
           (static_cast<uint16_t>(data[++i]) << 8);
}

uint16_t getValue(uint16_t v1, uint16_t v2)
{
    return v1 | (v2 << 8);
}




Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    initUi();
    initColorData();
    m_strPath = QApplication::applicationDirPath() + "/data.txt";
    ui->cmbBox_port->clear();
    ui->cmbBox_port->addItems(getSerialPortList());

    initPort();

    m_file.setFileName(m_strPath);
    m_file.open(QIODevice::WriteOnly);

    m_pTimer = new QTimer(this);
    connect(m_pTimer, &QTimer::timeout, this, &Widget::onTimeout);

    m_pAnim = new QVariantAnimation(this);
    m_pAnim->setDuration(100);
  //  connect(m_pAnim, &QVariantAnimation::valueChanged, this, &Widget::onAnimationValueChanged);

    QTimer::singleShot(100,this,[this](){
        initData();
        this->update();
        m_pTimer->start(100);
    });
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
    delete ui;
    m_file.close();
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
    if(m_serialPort.isOpen())
    {
        m_serialPort.clear();
        m_serialPort.close();
    }
    else
    {
        m_serialPort.setPortName(strSerialPort);
        m_serialPort.setBaudRate(QSerialPort::Baud115200, QSerialPort::AllDirections);
        m_serialPort.setDataBits(QSerialPort::Data8);
        m_serialPort.setFlowControl(QSerialPort::NoFlowControl);
        m_serialPort.setParity(QSerialPort::NoParity);
        m_serialPort.setStopBits(QSerialPort::OneStop);
        connect(&m_serialPort, &QSerialPort::readyRead,this,&Widget::onReciveData);
        if(!m_serialPort.open(QIODevice::ReadOnly))
        {
            return;
        }
    }
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
    m_file.write(strData);
    m_file.write("\n");

    if(strData.isEmpty() || strData.length() != 247)
        return;
    if(strData.at(0) == 0x3C && strData.at(1) == 0x02 && strData.at(strData.length()- 1) == 0x3E)
    {
        for(int i=0; i<9; ++i)
        {
            if(1 == i || 5 == i)
                continue;
            int sum = 0;
            for(int j= 0; j<12; j+=2)
            {
                sum += getValue(strData[i*13 + j], strData[i*13 + j+1]);
            }
            m_valueMap.insert(i, sum / 12);
            if(2 == i)
                m_valueMap.insert(1, sum / 12);
        }
        update();
    }
}

void Widget::paintEvent(QPaintEvent *event)
{
   // QWidget::paintEvent(event);

     QPainter painter(this);
     
     painter.setRenderHint(QPainter::Antialiasing, true);
     painter.eraseRect(rect());
    painter.fillRect(rect(), Qt::white);
    painter.drawImage(ui->label->geometry(), QImage(":/images/images/chair.png"));

    for(QMap<int, QRect>::iterator it = m_areaMap.begin(); it != m_areaMap.end(); ++it)
    {/*
        if (it != m_areaMap.begin())
            continue;*/

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
        int wStep = (rect.width() - 10) / colorCount;
        int hStep = (rect.height() - 30) / colorCount;
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
            QColor clr = m_colorMap.value(colorIndex);
            radialGrad.setColorAt(1 - j*step, m_colorMap.value(colorIndex));
            QPoint pt = QPoint(rect.x() + j * wStep, rect.y() + j * hStep);
            QRect tmprect = QRect(rect.x() + j * wStep / 2, rect.y() + j * hStep / 2, rect.width() - (j * wStep), rect.height() - (j * hStep));
         
            painter.setBrush(radialGrad); // 设置画笔的填充颜色为渐变
            painter.setPen(Qt::NoPen); // 不绘制边框
            painter.drawEllipse(tmprect); // 绘制椭圆，填充渐变色
         //   qDebug() << "tmpRect:" << tmprect<<"  colorIndex:"<< colorIndex << "   color:" << clr;
            --colorIndex;
            ++j;
            if (j >= colorCount)
                break;
        }

    }
  //  qDebug() << "update ----------------------------------------------------------------------------:";

}

void Widget::resizeEvent(QResizeEvent *e)
{
   // initData();s
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

    qDebug()<<m_areaMap[0];
    qDebug()<<"label width="<<ui->label->width()<<"  x="<<ui->label->geometry().x();

    for(int i=0; i<9; ++i)
    {
        if(1 == i || 5 == i)
            continue;
        int sum = 0;
        int randomValue = QRandomGenerator::global()->bounded(5, 11)*100;
        randomValue = 600;
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
    m_colorMap.insert(20, QColor(255, 255, 255, 0));
}

int Widget::getColorLevel(int value)
{
    if(1000 >= value     && 950 < value)
        return 0;
    else if(950 >= value && 900 <value)
        return 1;
    else if(900 >= value && 850 < value)
        return 2;
    else if(850 >= value && 800 < value)
        return 3;
    else if(800 >= value && 750 < value)
        return 4;
    else if(750 >= value && 700 < value)
        return 5;
    else if(700 >= value && 650 < value)
        return 6;
    else if(650 >= value && 600 < value)
        return 7;
    else if(600 >= value && 550 < value)
        return 8;
    else if (550 >= value && 500 < value)
        return 9;
    else if (500 >= value && 450 < value)
        return 10;
    else if (450 >= value && 400 < value)
        return 11;
    else if (400 >= value && 350 < value)
        return 12;
    else if (350 >= value && 300 < value)
        return 12;
    else if (300 >= value && 250 < value)
        return 14;
    else if (250 >= value && 200 < value)
        return 15;
    else if (200 >= value && 150 < value)
        return 16;
    else if (150 >= value && 100 < value)
        return 17;
    else if (100 >= value && 50 < value)
        return 18;
    else if (50 >= value && 0 < value)
        return 19;
    else
        return 20;
}

void Widget::on_btnOpen_clicked()
{
    initPort();
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
        if (value <= 0)
            m_pTimer->stop();

        QColor endColor = m_colorMap.value(getColorLevel(value));

        for (int aKey : m_animMap)
        {
            if (aKey == key)
            {
                m_animMap.key(aKey)->setStartValue(startColor);   // 红色
                m_animMap.key(aKey)->setEndValue(startColor);
                m_animMap.key(aKey)->start();
                break;
            }
        }
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

