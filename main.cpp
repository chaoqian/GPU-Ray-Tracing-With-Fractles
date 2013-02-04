
#include <QApplication>

#include <cstdlib>
#include <cstdio>

#include "MainWindow.hpp"

#ifdef _MSC_VER
const char * DEFAULT_IMAGE_PATH = "C:/Daniel/facu/CSCI1230/image/";
//const char * DEFAULT_IMAGE_PATH = "C:/facu/CSCI1230/image/";
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow win;

    win.show();

    return app.exec();
}