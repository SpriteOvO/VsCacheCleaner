#include "VsCacheCleaner.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VsCacheCleaner w;
    w.show();
    return a.exec();
}
