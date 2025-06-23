#ifndef COLORHELPER_H
#define COLORHELPER_H

#include <QObject>
#include <QColor>
#include <QMap>

class ColorHelper : public QObject
{
    Q_OBJECT
public:
    explicit ColorHelper(QObject *parent = nullptr);

    static void initColorData();
signals:

private:
    static QMap<int,QColor> m_colorMap;
};

#endif // COLORHELPER_H
