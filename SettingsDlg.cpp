#include "SettingsDlg.h"
#include <QGraphicsDropShadowEffect>

SettingsDlg::SettingsDlg(const ST_ViewParam& stParam, QWidget *parent)
	: QDialog(parent), m_stParam(stParam)
{
	ui.setupUi(this);

    initUi();
    updateUi(stParam);
}

SettingsDlg::~SettingsDlg()
{

}

void SettingsDlg::initUi()
{
    this->setWindowIcon(QIcon(":/images/images/pressure.png"));
    setAttribute(Qt::WA_TranslucentBackground, true);
   // ui.widget->setStyleSheet("background: #FFFFFF;"); // 如果需要样式表

    this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);
    //实例阴影shadow
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    //设置阴影距离
    shadow->setOffset(0, 0);
    //设置阴影颜色
    shadow->setColor(QColor(0, 0, 0, 76));
    //设置阴影圆角
    shadow->setBlurRadius(10);
    //给嵌套QWidget设置阴影
    this->setGraphicsEffect(shadow);

    ui.lineEdit->setValidator(new QIntValidator(ui.lineEdit));
    ui.lineEdit_2->setValidator(new QIntValidator(ui.lineEdit_2));
    
}

void SettingsDlg::updateUi(const ST_ViewParam& stParam)
{
    ui.lineEdit->setText(QString::number(stParam.nReadTime));
    ui.lineEdit_2->setText(QString::number(stParam.nDelayTime));
    ui.pushButton->setChecked(  !stParam.showValueMap.value(0));
    ui.pushButton_2->setChecked(!stParam.showValueMap.value(1));
    ui.pushButton_3->setChecked(!stParam.showValueMap.value(2));
    ui.pushButton_4->setChecked(!stParam.showValueMap.value(3));
    ui.pushButton_5->setChecked(!stParam.showValueMap.value(5));
    ui.pushButton_6->setChecked(!stParam.showValueMap.value(6));
    ui.pushButton_7->setChecked(!stParam.showValueMap.value(7));
    ui.pushButton_8->setChecked(!stParam.showValueMap.value(8));

    ui.comboBox->setCurrentIndex(stParam.indexMap.value(0));
    ui.comboBox_2->setCurrentIndex(stParam.indexMap.value(1));
    ui.comboBox_3->setCurrentIndex(stParam.indexMap.value(2));
    ui.comboBox_4->setCurrentIndex(stParam.indexMap.value(3));
    ui.comboBox_5->setCurrentIndex(stParam.indexMap.value(5));
    ui.comboBox_6->setCurrentIndex(stParam.indexMap.value(6));
    ui.comboBox_7->setCurrentIndex(stParam.indexMap.value(7));
    ui.comboBox_8->setCurrentIndex(stParam.indexMap.value(8));
}


void SettingsDlg::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void SettingsDlg::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void SettingsDlg::on_btnOK_clicked()
{
    updateParams();
    this->accept();
}

void SettingsDlg::on_btnCancel_clicked()
{
    this->reject();
}

void SettingsDlg::updateParams()
{
    m_stParam.nReadTime = ui.lineEdit->text().toInt();
    m_stParam.nDelayTime = ui.lineEdit_2->text().toInt();

    m_stParam.showValueMap[0] = !ui.pushButton->isChecked();
    m_stParam.showValueMap[1] = !ui.pushButton_2->isChecked();
    m_stParam.showValueMap[2] = !ui.pushButton_3->isChecked();
    m_stParam.showValueMap[3] = !ui.pushButton_4->isChecked();
    m_stParam.showValueMap[5] = !ui.pushButton_5->isChecked();
    m_stParam.showValueMap[6] = !ui.pushButton_6->isChecked();
    m_stParam.showValueMap[7] = !ui.pushButton_7->isChecked();
    m_stParam.showValueMap[8] = !ui.pushButton_8->isChecked();

    m_stParam.indexMap[0] = ui.comboBox->currentIndex();
    m_stParam.indexMap[1] = ui.comboBox_2->currentIndex();
    m_stParam.indexMap[2] = ui.comboBox_3->currentIndex();
    m_stParam.indexMap[3] = ui.comboBox_4->currentIndex();
    m_stParam.indexMap[5] = ui.comboBox_5->currentIndex();
    m_stParam.indexMap[6] = ui.comboBox_6->currentIndex();
    m_stParam.indexMap[7] = ui.comboBox_7->currentIndex();
    m_stParam.indexMap[8] = ui.comboBox_8->currentIndex();
}   