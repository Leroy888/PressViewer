#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFile>
#include <QRect>
#include <QMap>
#include <QMouseEvent>

class QTimer;
class QVariantAnimation;

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

    struct ST_Anim
    {

    };

protected:
    void initUi();
    void initPort();
    QStringList getSerialPortList();

    void paintEvent(QPaintEvent *event) override ;
    void resizeEvent(QResizeEvent* e) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void initData();
    void initColorData();
    int getColorLevel(int value);
    void initAnimMap();

protected slots:
    void onReciveData();
    void onTimeout();
    void onAnimationValueChanged(const QVariant& value);

private slots:
    void on_btnOpen_clicked();

    void on_btnClose_clicked();

private:
    Ui::Widget *ui;

    QSerialPort m_serialPort;
    QSerialPortInfo m_serialPortInfo;
    QString m_strPath;
    QFile m_file;
    QMap<int, QRect> m_areaMap;
    QMap<int, int> m_valueMap;
    QMap<int,QColor> m_colorMap;
    QPoint m_dragPosition;
    QTimer* m_pTimer;
    QVariantAnimation* m_pAnim;
    QMap< QVariantAnimation*, int> m_animMap;
};
#endif // WIDGET_H
