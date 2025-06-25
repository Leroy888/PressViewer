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

    // 设置输出信息格式
    QString strDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
    //保存信息：类型、消息、文件、行数、函数
    QString strMessage = QString("Date:%1   Type:%2  Message:%3 ").arg(strDateTime)
        .arg(strMsg.toLatin1().constData()).arg(localMsg.constData());

    // 输出信息至文件中（读写、追加形式）
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
