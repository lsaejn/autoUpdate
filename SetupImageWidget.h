#pragma once
#include <QListWidget>

//fix me
//这个QListWidget也没时间写了
//重设QListWidget，以便预设一些样式
class SetupImageWidget : public QListWidget
{
    Q_OBJECT
public:
    SetupImageWidget(QWidget* parent);
    void AdjustUIElement();
};
