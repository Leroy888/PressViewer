#include "ColorHelper.h"

QMap<int,QColor> ColorHelper::m_colorMap;

ColorHelper::ColorHelper(QObject *parent)
    : QObject{parent}
{

}

void ColorHelper::initColorData()
{
    m_colorMap.insert(0, QColor(222, 74, 63));
    m_colorMap.insert(1, QColor(222, 100, 63));
    m_colorMap.insert(2, QColor(222, 123, 63));
    m_colorMap.insert(3, QColor(222, 145, 63));

    m_colorMap.insert(4, QColor(222, 171, 63));
    m_colorMap.insert(5, QColor(222, 190, 63));
    m_colorMap.insert(6, QColor(237, 231, 69));
    m_colorMap.insert(7, QColor(237, 199, 69));
    m_colorMap.insert(8, QColor(215, 204, 82));

    m_colorMap.insert(9, QColor(207, 215, 82));
    m_colorMap.insert(10, QColor(193, 197, 80));
    m_colorMap.insert(11, QColor(160, 197, 80));
    m_colorMap.insert(12, QColor(119, 197, 80));
    m_colorMap.insert(13, QColor(80, 197, 116));
    m_colorMap.insert(14, QColor(80, 197, 141));

    m_colorMap.insert(15, QColor(80, 197, 185));
    m_colorMap.insert(16, QColor(80, 179, 197));
    m_colorMap.insert(17, QColor(77, 168, 211));
    m_colorMap.insert(18, QColor(77, 143, 211));
    m_colorMap.insert(19, QColor(130, 170, 229));
}
