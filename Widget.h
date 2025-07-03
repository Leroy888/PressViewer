#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFile>
#include <QRect>
#include <QMap>
#include <QMouseEvent>
#include <QThread>
#include "SettingsDlg.h"

class QTimer;
class QVariantAnimation;
class SerialWorker;

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
    QStringList getSerialPortList();

    void paintEvent(QPaintEvent *event) override ;
    void resizeEvent(QResizeEvent* e) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void initData();
    void initColorData();
    void initValues();
    void initAnimMap();
    void repaintColor();

    void drawRoundedRectTopLeft(QPainter *painter, const QRect &rect, int radius);
    void drawRoundedRectTopRight(QPainter *painter, const QRect &rect, int radius);
protected slots:
    void onTimeout();
    void onAnimationValueChanged(const QVariant& value);
    void onUpdateData(QMap<int, int> valueMap);
    void onPortOpened(bool success);
    void onUpdateColor(QMap<int,QList<QColor>> realColorMap);

private slots:
    void on_btnOpen_clicked();

    void on_btnClose_clicked();

    void on_btnClosePort_clicked();

    void on_btnSettings_clicked();

    void on_btnMin_clicked();

signals:
    void sigOpenPort();
    void sigClosePort();

private:
    Ui::Widget *ui;

    QSerialPort m_serialPort;
    QSerialPortInfo m_serialPortInfo;
    QMap<int, QRect> m_areaMap;
    QMap<int, int> m_valueMap;
    QMap<int, int> m_curValueMap;
    QMap<int,QColor> m_colorMap;
    QPoint m_dragPosition;
    QTimer* m_pTimer;
    QVariantAnimation* m_pAnim;
    QMap< QVariantAnimation*, int> m_animMap;
    QMap< int, QColor> m_curColorMap;

    QThread* m_serialThread;
    SerialWorker* m_serialWorker;
    bool m_bOpened;

    ST_ViewParam m_stParams;

    QMap<int,QList<QColor>> m_realColorMap;
};
#endif // WIDGET_H
