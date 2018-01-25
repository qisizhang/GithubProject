#include "drawgerimage.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    drawGerImage w;
    w.show();

    return a.exec();
}
