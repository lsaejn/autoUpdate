#include "QFrameLessWidget_Alime.h"
#include "Alime_TransparentWidget.h"
#include "Alime/ScopeGuard.h"
#include "QssLoader.h"
#include "AppUtility.h"
#include "ConfigFileRW.h"

#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QtWidgets/QApplication>
#include <QFontDatabase>

/*
�����ǵ�����ҵ������Ҫ�Ѵ��뾡��д�ü򵥡�
����û����ƣ�����ֱ�Ӷ��Űٶ�����д��
����ģ���ǲο�¥�µĲ�����������
������»������Ǹ�������ģ��Ĵ��룬�������ļ���αװ�ɸ������ذ���

������µ���������:
�����Ʋ�ѯ�ļ�����Ҫ������id�ģ������Ӧ�����ڿͻ��˴����
����ѡ��ķ����ǣ�дһ���ļ��б��ļ�����¼���·����MD5����,��Ҳ����֤һ���Եķ�ʽ����������֤��

��ô���̺������:
����ͻ��˰汾��V5.1.0,��ȡĳ���汾V5.2.2���ļ��б�
���㱾���ļ�MD5���ó�Ҫ���µ��ļ�����������������(���涼���Բ��ģ��̳�downloadInfo�����дһ���༴��)
ֻ��Ҫʵ�ֿ�ʼ��ͣɾ����Щ�ӿڣ�ʣ�µ�͸����
ps:����Ҫһ�������б�ָ��ִ�в������ʱӦִ����Щexe(����ע��)����Щ�������ļ�(������)
����ʱ������Ҫ���ݳ�һ���ļ��У����ڱ�����һ�ε��ļ������ڻع���(����Ǻ����ƻ�)
*/

/*
�����޸���Material���룬�Ա��޸�ripple����ɫ/����/����ʱ���
*/


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    if (argc == 2)
    {
        g_hwnd = (HWND)std::stoi(argv[1]);
    }
        
    QssLoader loder(GetStyleName(), app);

    CreateFolderForApp();
    qInstallMessageHandler(Logging);

    ALIME_SCOPE_EXIT{
        if (g_handle != INVALID_HANDLE_VALUE)
            CloseHandle(g_handle);
    };

    if (!IsInstanceOn())
    {
        QMessageBox::critical(NULL, u8"�ظ���", u8"������������");
        return -1;
    }
       
    Alime_TransparentWidget w;
    w.show();
    return app.exec();
}
