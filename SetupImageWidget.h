#pragma once
#include <QListWidget>

//fix me
//���QListWidgetҲûʱ��д��
//����QListWidget���Ա�Ԥ��һЩ��ʽ
class SetupImageWidget : public QListWidget
{
    Q_OBJECT
public:
    SetupImageWidget(QWidget* parent);
    void AdjustUIElement();
};
