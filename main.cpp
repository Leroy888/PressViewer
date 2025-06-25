#include "Widget.h"

#include <QApplication>
#include <QFile>
#include <QtDebug >
#include <QFile >
#include <QTextStream >
#include <QDateTime>


void logOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QString strMsg("");
    switch (type)
    {
    case QtDebugMsg:
        strMsg = QString("Debug:");
        break;
    case QtWarningMsg:
        strMsg = QString("Warning:");
        break;
    case QtCriticalMsg:
        strMsg = QString("Critical:");
        break;
    case QtFatalMsg:
        strMsg = QString("Fatal:");
        break;
    }

    if (QtDebugMsg != type)
        return;

    // ���������Ϣ��ʽ
    QString strDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
    //������Ϣ�����͡���Ϣ���ļ�������������
    QString strMessage = QString("Date:%1   Type:%2  Message:%3 ").arg(strDateTime)
        .arg(strMsg.toLatin1().constData()).arg(localMsg.constData());

    // �����Ϣ���ļ��У���д��׷����ʽ��
    QFile file(QApplication::applicationDirPath() + "/log.txt");
    file.open(QIODevice::ReadWrite | QIODevice::Append);
    QTextStream stream(&file);
    stream << strMessage << "\r\n";
    file.flush();
    file.close();
}


int main(int argc, char *argv[])
{
    qInstallMessageHandler(logOutput);
    QApplication a(argc, argv);

    Widget w;
    w.show();
    return a.exec();
}
