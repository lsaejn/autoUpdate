#pragma once
#include <QListWidget>

//fix me
//���QListWidgetҲûʱ��д��
class SetupImageWidget : public QListWidget
{
    Q_OBJECT
public:
    SetupImageWidget(QWidget* parent);
    void AdjustUIElement();
};
