#include "QFrameLessWidget_Alime.h"
#include "Alime_TransparentWidget.h"
#include "Alime/ScopeGuard.h"
#include "QssLoader.h"
#include "AppUtility.h"

#include <QtWidgets/QApplication>
#include <QFile>
#include <QDebug>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QssLoader loder(":/qss/dark.qss", app);

    qInstallMessageHandler(Logging);

    ALIME_SCOPE_EXIT{
        if (g_handle != INVALID_HANDLE_VALUE)
            CloseHandle(g_handle);
    };

    if (!IsInstanceOn())
    {
        QMessageBox::critical(NULL, u8"重复打开", u8"程序正在运行");
        return -1;
    }
        
    //qDebug() << "fuck";
    //qWarning() << "off";
    //qCritical() << "mother";
    //qFatal() << "fucker";
    Alime_TransparentWidget w;
    w.show();
    return app.exec();
}
