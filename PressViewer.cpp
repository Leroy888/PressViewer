#include "PressViewer.h"
#include <QApplication>
#include <QPainter>
#include <QScreen>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QDebug>
#include <QStyleOption>

PressViewer::PressViewer()
{
    initUi();
    m_strPath = QApplication::applicationDirPath() + "/data.txt";
    // ui->cmbBox_port->clear();
    // ui->cmbBox_port->addItems(getSerialPortList());

    initPort();

    m_file.setFileName(m_strPath);
    m_file.open(QIODevice::WriteOnly);

    QTimer::singleShot(100,this,[this](){
        initData();
        this->update();
    });
}

PressViewer::~PressViewer()
{
    m_file.close();
}

uint16_t PressViewer::readU16LittleEndian(const uint8_t* data, int i)
{
    return static_cast<uint16_t>(data[i]) |
           (static_cast<uint16_t>(data[++i]) << 8);
}

uint16_t PressViewer::getValue(uint16_t v1, uint16_t v2)
{
    return v1 | (v2 << 8);
}


void PressViewer::initUi()
{
    setAttribute(Qt::WA_TranslucentBackground); // 推荐添加

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

    this->setFixedSize(810, 752);
    QFrame* pFrame = new QFrame(this);
    pFrame->setFixedSize(800, 742);
    pFrame->move(5, 5);
    m_pLabel = new QLabel(pFrame);
    m_pLabel->setFixedSize(468, 700);
    m_pLabel->move(115, 32);

}

void PressViewer::initPort()
{
    QString strSerialPort /*= ui->cmbBox_port->currentText()*/;
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
        connect(&m_serialPort, &QSerialPort::readyRead,this,&PressViewer::onReciveData);
        if(!m_serialPort.open(QIODevice::ReadOnly))
        {
            return;
        }
    }
}

QStringList PressViewer::getSerialPortList()
{
    QStringList serialPortList;
    for(const auto& port : QSerialPortInfo::availablePorts())
    {
        serialPortList.push_back(port.portName());
    }

    return serialPortList;
}

void PressViewer::onReciveData()
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

void PressViewer::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
    QPainter painter(this);

    // QRadialGradient radialGrad(m_pLabel->pos().x() + m_pLabel->width()/2, m_pLabel->pos().y() + m_pLabel->height()/2, m_pLabel->width()/2); // 创建一个径向渐变，中心在中心，半径为50
    // radialGrad.setColorAt(0.0, Qt::red); // 设置起始颜色为黄色
    // radialGrad.setColorAt(0.1, QColor(255, 100, 220)); // 设置起始颜色为黄色
    // radialGrad.setColorAt(0.2, QColor(255, 100, 220)); // 设置起始颜色为黄色
    // radialGrad.setColorAt(0.4, QColor(163, 126, 220)); // 设置起始颜色为黄色
    // radialGrad.setColorAt(0.6, QColor(182, 175, 220)); // 设置起始颜色为黄色
    // radialGrad.setColorAt(0.8, QColor(255, 220, 220)); // 设置起始颜色为黄色

    // radialGrad.setColorAt(1.0, Qt::white); // 设置结束颜色为绿色
    // painter.setBrush(radialGrad); // 设置画笔的填充颜色为渐变
    // painter.setPen(Qt::NoPen); // 不绘制边框
    // painter.drawEllipse(m_pLabel->pos().x(), m_pLabel->pos().y(), m_pLabel->width(), m_pLabel->height()); // 绘制椭圆，填充渐变色

    painter.fillRect(rect(), Qt::white); // 自定义背景颜色
    painter.drawImage(m_pLabel->geometry(), QImage(":/images/images/chair.png"));

    for(QMap<int, QRect>::iterator it = m_areaMap.begin(); it != m_areaMap.end(); ++it)
    {
        QRect rect = it.value();
        int key = it.key();
        static bool bFirst = true;
        QRect rct = m_pLabel->geometry();
        if(bFirst)
        {
            qDebug()<<"rect:"<<rect;
            qDebug()<<"rct:"<<rct;
            bFirst = false;
        }
        QRadialGradient radialGrad(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2,
                                   rect.width() > rect.height() ? rect.width() / 2 : rect.height() / 2);

        radialGrad.setColorAt(0.0, Qt::red); // 设置起始颜色为黄色
        radialGrad.setColorAt(0.1, QColor(255, 100, 220)); // 设置起始颜色为黄色
        radialGrad.setColorAt(0.2, QColor(255, 100, 220)); // 设置起始颜色为黄色
        radialGrad.setColorAt(0.4, QColor(163, 126, 220)); // 设置起始颜色为黄色
        radialGrad.setColorAt(0.6, QColor(182, 175, 220)); // 设置起始颜色为黄色
        radialGrad.setColorAt(0.8, QColor(255, 220, 220)); // 设置起始颜色为黄色
        radialGrad.setColorAt(1.0, Qt::white); // 设置结束颜色为绿色

        int value = m_valueMap.value(key);
        int level = getColorLevel(value);
        float step = 1.0 / (m_valueMap.size() - level);
        radialGrad.setColorAt(0.0, m_colorMap.value(level));
        for(int i=level; i < m_valueMap.size(); i++)
        {
            radialGrad.setColorAt(i*step, m_colorMap.value(i));
        }

        painter.setBrush(radialGrad); // 设置画笔的填充颜色为渐变
        painter.setPen(Qt::NoPen); // 不绘制边框
        painter.drawEllipse(rect); // 绘制椭圆，填充渐变色
    }


}

void PressViewer::resizeEvent(QResizeEvent *e)
{
    initData();
}

void PressViewer::initData()
{
    QRect rect = QRect((m_pLabel->geometry().x() +  m_pLabel->width() * 0.49) ,
                       (m_pLabel->geometry().y() +  m_pLabel->height() / 4), m_pLabel->width() * 0.107, m_pLabel->height() * 0.185);
    m_areaMap.insert(0, rect);
    rect = QRect((m_pLabel->geometry().x() +  m_pLabel->width() * 0.49) ,
                 m_pLabel->height() * 0.5, m_pLabel->width() * 0.107, m_pLabel->height() * 0.128);
    m_areaMap.insert(3, rect);
    rect = QRect((m_pLabel->geometry().x() +  m_pLabel->width() * 0.37) ,
                 (m_pLabel->geometry().y() +  m_pLabel->height() * 0.37), m_pLabel->width() * 0.053, m_pLabel->height() * 0.164);
    m_areaMap.insert(5, rect);
    rect = QRect((m_pLabel->geometry().x() +  m_pLabel->width() * 0.67) ,
                 (m_pLabel->geometry().y() +  m_pLabel->height() * 0.37), m_pLabel->width() * 0.053, m_pLabel->height() * 0.164);
    m_areaMap.insert(6, rect);
    rect = QRect((m_pLabel->geometry().x() +  m_pLabel->width() * 0.41) ,
                 (m_pLabel->geometry().y() +  m_pLabel->height() * 0.6), m_pLabel->width() * 0.107, m_pLabel->height() * 0.107);
    m_areaMap.insert(7, rect);
    rect = QRect((m_pLabel->geometry().x() +  m_pLabel->width() * 0.57) ,
                 (m_pLabel->geometry().y() +  m_pLabel->height() * 0.6), m_pLabel->width() * 0.107, m_pLabel->height() * 0.107);
    m_areaMap.insert(8, rect);
    rect = QRect((m_pLabel->geometry().x() +  m_pLabel->width() * 0.37) ,
                 (m_pLabel->geometry().y() +  m_pLabel->height() * 0.72), m_pLabel->width() * 0.107, m_pLabel->height() * 0.1);
    m_areaMap.insert(1, rect);
    rect = QRect((m_pLabel->geometry().x() +  m_pLabel->width() * 0.60) ,
                 (m_pLabel->geometry().y() +  m_pLabel->height() * 0.72), m_pLabel->width() * 0.107, m_pLabel->height() * 0.1);
    m_areaMap.insert(2, rect);

    qDebug()<<m_areaMap[0];
    qDebug()<<"label width="<<m_pLabel->width()<<"  x="<<m_pLabel->geometry().x();
}

void PressViewer::initColorData()
{
    m_colorMap.insert(0, QColor());
    m_colorMap.insert(1, QColor());
    m_colorMap.insert(2, QColor());
    m_colorMap.insert(3, QColor());
    m_colorMap.insert(4, QColor());
    m_colorMap.insert(5, QColor());
    m_colorMap.insert(6, QColor());
    m_colorMap.insert(7, QColor());
}

int PressViewer::getColorLevel(int value)
{
    if(1000 >= value     && 900 < value)
        return 0;
    else if(900 >= value && 800 <value)
        return 1;
    else if(800 >= value && 700 < value)
        return 2;
    else if(700 >= value && 600 < value)
        return 3;
    else if(600 >= value && 500 < value)
        return 4;
    else if(500 >= value && 400 < value)
        return 5;
    else if(400 >= value && 300 < value)
        return 6;
    else if(300 >= value && 200 < value)
        return 7;
    else if(200 >= value && 100 < value)
        return 8;
    else
        return 9;
}

void PressViewer::on_btnOpen_clicked()
{
    initPort();
}

