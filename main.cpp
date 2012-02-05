#include <QtGui/QApplication>
#include "lirch_qt_interface.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LirchQtInterface w;
    w.show();
    return a.exec();
}
