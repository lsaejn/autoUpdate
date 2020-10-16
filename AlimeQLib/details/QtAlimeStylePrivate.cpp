#include "QtAlimeStylePrivate.h"
#include <QFontDatabase>
#include "../QtAlimeTheme.h"
#include "../QtAlimeStyle.h"

void QtAlimeStylePrivate::init()
{
    Q_Q(QtAlimeStyle);

    QFontDatabase::addApplicationFont(":/fonts/roboto_regular");
    QFontDatabase::addApplicationFont(":/fonts/roboto_medium");
    QFontDatabase::addApplicationFont(":/fonts/roboto_bold");

    q->setTheme(new QtAlimeTheme);
}