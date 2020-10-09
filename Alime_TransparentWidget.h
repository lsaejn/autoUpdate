#pragma once
#include <QWidget>

/*
* ���õ��ޱ߿���Ӱ���������鿴�ҵ�github
* ���ڴ���ĵòִ٣�һʱ��Ҳ�治����
* ��������/С��������ťhover״̬�쳣
*/
/*
--------------Alime_TransparentWidget---
|-------------Alime_WindowBase----------|
|-------------Alime_TitleBar---------------.|
|-------------Alime_ContentWidget  ------|
|          |                                                    |
| -left--| ----------right-------------------  |
|          |                                                    |
|------------------------------------------^|
*/

class Alime_TransparentWidget : public QWidget
{
    Q_OBJECT

public:
    Alime_TransparentWidget(QWidget* parent = Q_NULLPTR);
    virtual bool nativeEvent(const QByteArray& eventType, void* message, long* result);
private:
    int boundaryWidth;
};
