#include "ioser123.h"
#include "ioser123application.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    IOSer123Application a(argc, argv);
    IOSer123 w;

    a.w = &w;

    w.show();

    return a.exec();
}
