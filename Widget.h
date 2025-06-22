#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();


protected:
    void initUi();
    void initPort();
    QStringList getSerialPortList();

    void paintEvent(QPaintEvent *event) override ;
    void resizeEvent(QResizeEvent* e) override;
    void initData();
    void initColorData();
    int getColorLevel(int value);

protected slots:
    void onReciveData();

private slots:
    void on_btnOpen_clicked();

private:
    Ui::Widget *ui;

    QSerialPort m_serialPort;
    QSerialPortInfo m_serialPortInfo;
    QString m_strPath;
    QFile m_file;
    QMap<int, QRect> m_areaMap;
    QMap<int, int> m_valueMap;
    float m_scale;
    QMap<int,QColor> m_colorMap;
};
#endif // WIDGET_H
