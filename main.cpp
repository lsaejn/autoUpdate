#include "QFrameLessWidget_Alime.h"
#include "Alime_TransparentWidget.h"
#include "Alime/ScopeGuard.h"
#include "Alime/Console.h"
#include "QssLoader.h"
#include "AppUtility.h"
#include "ConfigFileRW.h"

#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QtWidgets/QApplication>
#include <QFontDatabase>

/*
由于是单人作业，我需要把代码尽量写得简单。
界面没人设计，所以直接对着百度网盘写。
下载模块是参考楼下的补丁包方案。
差异更新基本上是复用下载模块的代码，将更新文件夹伪装成更新下载包。

差异更新的问题在于:
阿里云查询文件是需要开发者id的，这个不应该置于客户端代码里。
所以选择的方案是，写一个文件列表文件，记录相对路径和MD5即可,这也是验证一致性的方式，很容易验证。

那么流程很清楚了:
假设客户端版本是V5.1.0,读取某个版本V5.2.2的文件列表
计算本地文件MD5，得出要更新的文件。丢到差异更新类里。(界面都可以不改，继承downloadInfo类或重写一个类即可)
只需要实现开始暂停删除这些接口，剩下的透明。
ps:还需要一个过滤列表，指明执行差异更新时应执行哪些exe(用于注册)，哪些是配置文件(不更新)
更新时，还需要备份出一个文件夹，用于保存上一次的文件，用于回滚。(这个是后续计划)
*/

/*
我们修改了Material代码，以便修改ripple的颜色/坐标/持续时间等
*/


int main(int argc, char *argv[])
{
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "1");
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);
    if (argc == 2)
    {
        g_hwnd = (HWND)std::stoi(argv[1]);
    }
    Alime::Console::CreateConsole();
    
    
    Alime::Console::SetWindowSize(400, 800);
    Alime::Console::SetWindowPosition(1500, 0);
        
    QssLoader loder(GetStyleName(), app);

    CreateFolderForApp();
    qInstallMessageHandler(Logging);

    ALIME_SCOPE_EXIT{
        if (g_handle != INVALID_HANDLE_VALUE)
            CloseHandle(g_handle);
    };

    if (!IsMutexNew())
    {
        QMessageBox::critical(NULL, u8"重复打开", u8"程序正在运行");
        return -1;
    }
       
    Alime_TransparentWidget w;
    w.show();
    return app.exec();
}
