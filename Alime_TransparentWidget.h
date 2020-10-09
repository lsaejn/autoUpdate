#pragma once
#include <QWidget>

/*
* 更好的无边框阴影解决方案请查看我的github
* 由于代码改得仓促，一时间也替不过来
* 解决了最大/小化慢，按钮hover状态异常
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
