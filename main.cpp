#include "QFrameLessWidget_Alime.h"
#include "Alime_TransparentWidget.h"
#include "QssLoader.h"
#include <QtWidgets/QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QssLoader loder(":/qss/dark.qss", app);
    Alime_TransparentWidget w;
    w.show();
    return app.exec();
}
