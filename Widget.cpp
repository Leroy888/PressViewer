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
#include "SettingsDlg.h"


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget),
    m_serialThread(new QThread(this)),
    m_serialWorker(new SerialWorker),
    m_bOpened(false)
{
    ui->setupUi(this);

    initUi();
    initColorData();
    ui->cmbBox_port->clear();
    ui->cmbBox_port->addItems(getSerialPortList());

    connect(m_serialWorker, &SerialWorker::sigUpdateData, this, &Widget::onUpdateData);
    connect(m_serialWorker, &SerialWorker::sigUpdateColor, this, &Widget::onUpdateColor);
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

    QTimer::singleShot(100,this,[this](){
        initData();
        initValues();
        m_curValueMap = m_valueMap;
        this->update();
       // m_pTimer->start(100);
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

    this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);
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

    ui->btnClosePort->setVisible(false);
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

void Widget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

     QPainter painter(this);
     
     painter.setRenderHint(QPainter::Antialiasing, true);
   //  painter.eraseRect(rect());
    painter.fillRect(this->rect(), Qt::gray);
    painter.fillRect(ui->widget->geometry(), Qt::white);
    painter.drawImage(ui->label->geometry(), QImage(":/images/images/chair.png"));

    // if(!m_bOpened)
    //     return;
    for(QMap<int, QRect>::iterator it = m_areaMap.begin(); it != m_areaMap.end(); ++it)
    {
        // if (it != m_areaMap.begin())
        //     continue;

        int key = it.key();
        if (!m_stParams.showValueMap.value(key))
            continue;

        QRect rect = it.value();
        int radio = rect.width() > rect.height() ? rect.width() / 2 : rect.height() / 2;
        QRadialGradient radialGrad(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2,
                                    rect.width() > rect.height() ? rect.width() / 2 : rect.height() / 2);

        int value = m_valueMap.value(key);
        int level = PublicFunc::getColorLevel(value);
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
            qreal xRadius = tmprect.width() * 0.4;          // 水平方向圆角半径
            qreal yRadius = tmprect.height() * 0.2;
            painter.setBrush(radialGrad); // 设置画笔的填充颜色为渐变
            painter.setPen(Qt::NoPen); // 不绘制边框
            painter.drawRoundedRect(tmprect, xRadius, yRadius); // 绘制椭圆，填充渐变色
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
    //后背
    QRect rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.4405) ,
                       (ui->label->geometry().y() +  ui->label->height() * 0.25), ui->label->width() * 0.21, ui->label->height() * 0.11);
    m_areaMap.insert(0, rect);

    //腰部
    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.4412) ,
                 ui->label->height() * 0.413, ui->label->width() * 0.205, ui->label->height() * 0.205);
    m_areaMap.insert(3, rect);
    //右边
    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.39) ,
                 (ui->label->geometry().y() +  ui->label->height() * 0.38), ui->label->width() * 0.043, ui->label->height() * 0.18);
    m_areaMap.insert(5, rect);
    //左边
    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.66) ,
                 (ui->label->geometry().y() +  ui->label->height() * 0.38), ui->label->width() * 0.043, ui->label->height() * 0.18);
    m_areaMap.insert(6, rect);
    //右边臀部
    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.37) ,
                 (ui->label->geometry().y() +  ui->label->height() * 0.61), ui->label->width() * 0.175, ui->label->height() * 0.11);
    m_areaMap.insert(7, rect);
    //左边臀部
    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.55) ,
                 (ui->label->geometry().y() +  ui->label->height() * 0.61), ui->label->width() * 0.175, ui->label->height() * 0.11);
    m_areaMap.insert(8, rect);
    //右腿
    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.34) ,
                 (ui->label->geometry().y() +  ui->label->height() * 0.725), ui->label->width() * 0.195, ui->label->height() * 0.11);
    m_areaMap.insert(1, rect);
    //左腿
    rect = QRect((ui->label->geometry().x() +  ui->label->width() * 0.55) ,
                 (ui->label->geometry().y() +  ui->label->height() * 0.725), ui->label->width() * 0.195, ui->label->height() * 0.11);
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

void Widget::initValues()
{
    int value = 600;
    m_valueMap.insert(0, value);
    m_valueMap.insert(3, value);
    m_valueMap.insert(5, value);
    m_valueMap.insert(6, value);
    m_valueMap.insert(7, value);
    m_valueMap.insert(8, value);
    m_valueMap.insert(1, value);
    m_valueMap.insert(2, value);

    m_stParams.indexMap.insert(0, 5);
    m_stParams.indexMap.insert(1, 4);
    m_stParams.indexMap.insert(2, 4);
    m_stParams.indexMap.insert(3, 0);
    m_stParams.indexMap.insert(5, 0);
    m_stParams.indexMap.insert(6, 0);
    m_stParams.indexMap.insert(7, 11);
    m_stParams.indexMap.insert(8, 11);

    m_stParams.showValueMap.insert(0, true);
    m_stParams.showValueMap.insert(1, true);
    m_stParams.showValueMap.insert(2, true);
    m_stParams.showValueMap.insert(3, true);
    m_stParams.showValueMap.insert(5, true);
    m_stParams.showValueMap.insert(6, true);
    m_stParams.showValueMap.insert(7, true);
    m_stParams.showValueMap.insert(8, true);
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
    static int num = 0;
m_animMap.clear();
    for (QMap<int, int>::iterator it = m_valueMap.begin(); it != m_valueMap.end(); ++it)
    {
        int value = it.value();
        QColor startColor = m_colorMap.value(PublicFunc::getColorLevel(value));

       // value -= 30;
        int key = it.key();

        m_valueMap[key] = value;
        if (value <= 100)
            m_pTimer->stop();
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
    qDebug()<<"onAnimationValueChanged color:"<<c;
}

void Widget::onUpdateData(QMap<int, int> valueMap)
{
    //m_valueMap = valueMap;
    m_valueMap = m_curValueMap;
    m_curValueMap = valueMap;
    //m_pTimer->start(100);
   // qDebug()<<"update values ----------------------";
    this->update();

   // onTimeout();
}

void Widget::onPortOpened(bool success)
{
    if (!success)
    {
        QMessageBox::information(this, tr("Tips"), tr("Failed to open the serialport"));
        return;
    }
    else
    {
        ui->btnClosePort->setVisible(true);
        ui->btnOpen->setVisible(false);
    }
    m_bOpened = success;
}

void Widget::onUpdateColor(QMap<int, QList<QColor> > realColorMap)
{
    m_realColorMap = realColorMap;
    update();
}

void Widget::on_btnClosePort_clicked()
{
    emit sigClosePort();
    ui->btnOpen->setVisible(true);
    ui->btnClosePort->setVisible(false);

    m_valueMap.insert(0, 100);
    m_valueMap.insert(3, 100);
    m_valueMap.insert(5, 100);
    m_valueMap.insert(6, 100);
    m_valueMap.insert(7, 100);
    m_valueMap.insert(8, 100);
    m_valueMap.insert(1, 100);
    m_valueMap.insert(2, 100);

    update();
}

void Widget::on_btnSettings_clicked()
{
    SettingsDlg dlg(m_stParams, this);
    if(QDialog::Accepted == dlg.exec())
    {
        m_stParams = dlg.GetParams();
        m_serialWorker->SetParams(m_stParams);
    }
}


void Widget::on_btnMin_clicked()
{
    this->showMinimized();
}

void Widget::repaintColor()
{
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(this->rect(), Qt::gray);
    painter.fillRect(ui->widget->geometry(), Qt::white);
    painter.drawImage(ui->label->geometry(), QImage(":/images/images/chair.png"));

    if(!m_bOpened)
        return;
    for(QMap<int, QRect>::iterator it = m_areaMap.begin(); it != m_areaMap.end(); ++it)
    {
        int key = it.key();
        if (!m_stParams.showValueMap.value(key))
            continue;

        QRect rect = it.value();
        QRadialGradient radialGrad(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2,
                                   rect.width() > rect.height() ? rect.width() / 2 : rect.height() / 2);

        int value = m_valueMap.value(key);
        int level = PublicFunc::getColorLevel(value);
        float step = 1.0 / (m_colorMap.size() - level);
        int wStep = (rect.width() - rect.width() * 0.3) / 4;
        int hStep = (rect.height() - rect.height() * 0.3) / 4;
        radialGrad.setColorAt(0.0, m_colorMap.value(level));
        int j = 0;

        QList<QColor> clrList = m_realColorMap.value(key);
        for(int i=0; i < clrList.size(); i++)
        {
            QColor clr = clrList.at(i);
            radialGrad.setColorAt(1 - j*step, clr);
            QPoint pt = QPoint(rect.x() + j * wStep, rect.y() + j * hStep);
            QRect tmprect = QRect(rect.x() + j * wStep / 2, rect.y() + j * hStep / 2, rect.width() - (j * wStep), rect.height() - (j * hStep));

            painter.setBrush(radialGrad); // 设置画笔的填充颜色为渐变
            painter.setPen(Qt::NoPen); // 不绘制边框
            painter.drawEllipse(tmprect); // 绘制椭圆，填充渐变色
        }

    }
}

