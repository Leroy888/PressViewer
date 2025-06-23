#include "Widget.h"

#include <QApplication>
#include <QFile>
#include "PressViewer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Widget w;
    w.show();
    return a.exec();
}
