QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ColorHelper.cpp \
    PublicFunc.cpp \
    SerialWorker.cpp \
    SettingsDlg.cpp \
    main.cpp \
    Widget.cpp

HEADERS += \
    ColorHelper.h \
    Func.h \
    PublicFunc.h \
    SerialWorker.h \
    SettingsDlg.h \
    Widget.h

FORMS += \
    SettingsDlg.ui \
    Widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = logo.png
RC_FILE += Resource.rc


RESOURCES += \
    res/Res.qrc

DISTFILES += \
    res/etc/qt.conf

