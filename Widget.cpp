#include "Widget.h"
#include "ui_Widget.h"
#include <QApplication>
#include <QPainter>
#include <QScreen>
#include <QTimer>
#include <QDebug>
#include <windows.h>
#include <wingdi.h>

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


    m_strPath = QApplication::applicationDirPath() + "/data.txt";
    ui->cmbBox_port->clear();
    ui->cmbBox_port->addItems(getSerialPortList());

    initPort();

    m_file.setFileName(m_strPath);
    m_file.open(QIODevice::WriteOnly);

    QTimer::singleShot(100,this,[this](){
        initData();
        this->update();
    });
}

Widget::~Widget()
{
    delete ui;
    m_file.close();
}

void Widget::initUi()
{

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
     QPainter painter(this);
    // QRadialGradient radialGrad(ui->label->pos().x() + ui->label->width()/2, ui->label->pos().y() + ui->label->height()/2, ui->label->width()/2); // 创建一个径向渐变，中心在中心，半径为50
    // radialGrad.setColorAt(0.0, Qt::red); // 设置起始颜色为黄色
    // radialGrad.setColorAt(0.1, QColor(255, 100, 220)); // 设置起始颜色为黄色
    // radialGrad.setColorAt(0.2, QColor(255, 100, 220)); // 设置起始颜色为黄色
    // radialGrad.setColorAt(0.4, QColor(163, 126, 220)); // 设置起始颜色为黄色
    // radialGrad.setColorAt(0.6, QColor(182, 175, 220)); // 设置起始颜色为黄色
    // radialGrad.setColorAt(0.8, QColor(255, 220, 220)); // 设置起始颜色为黄色

    // radialGrad.setColorAt(1.0, Qt::white); // 设置结束颜色为绿色
    // painter.setBrush(radialGrad); // 设置画笔的填充颜色为渐变
    // painter.setPen(Qt::NoPen); // 不绘制边框
    // painter.drawEllipse(ui->label->pos().x(), ui->label->pos().y(), ui->label->width(), ui->label->height()); // 绘制椭圆，填充渐变色

    painter.drawImage(ui->label->geometry(), QImage(":/images/images/chair.png"));

    for(QMap<int, QRect>::iterator it = m_areaMap.begin(); it != m_areaMap.end(); ++it)
    {
        QRect rect = it.value();
        int key = it.key();
         static bool bFirst = true;
         QRect rct = ui->label->geometry();
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

void Widget::resizeEvent(QResizeEvent *e)
{
    initData();
}

void Widget::initData()
{
    m_scale = 2;
    qDebug()<<"DPI="<<m_scale;
    QRect rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.49) ,
                       (ui->label->geometry().y() +  ui->label->height() / 4), 100 / m_scale, 260 / m_scale);
    m_areaMap.insert(0, rect);
    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.49) ,
                 ui->label->height() * 0.5, 100 / m_scale, 180 / m_scale);
    m_areaMap.insert(3, rect);
    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.37) ,
                 (ui->label->geometry().y() +  ui->label->height() * 0.37), 50 / m_scale, 230 / m_scale);
    m_areaMap.insert(5, rect);
    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.67) ,
                 (ui->label->geometry().y() +  ui->label->height() * 0.37), 50 / m_scale, 230 / m_scale);
    m_areaMap.insert(6, rect);
    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.41) ,
                 (ui->label->geometry().y() +  ui->label->height() * 0.6), 100 / m_scale, 150 / m_scale);
    m_areaMap.insert(7, rect);
    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.57) ,
                 (ui->label->geometry().y() +  ui->label->height() * 0.6), 100 / m_scale, 150 / m_scale);
    m_areaMap.insert(8, rect);
    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.37) ,
                 (ui->label->geometry().y() +  ui->label->height() * 0.72), 100 / m_scale, 140 / m_scale);
    m_areaMap.insert(1, rect);
    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.60) ,
                 (ui->label->geometry().y() +  ui->label->height() * 0.72), 100 / m_scale, 140 / m_scale);
    m_areaMap.insert(2, rect);

    qDebug()<<m_areaMap[0];
    qDebug()<<"label width="<<ui->label->width()<<"  x="<<ui->label->geometry().x();
}

void Widget::initColorData()
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

int Widget::getColorLevel(int value)
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

void Widget::on_btnOpen_clicked()
{
    initPort();
}

