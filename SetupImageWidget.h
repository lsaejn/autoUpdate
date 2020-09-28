#pragma once
#include <QListWidget>

class SetupImageWidget : public QListWidget
{
    Q_OBJECT
public:
    SetupImageWidget(QWidget* parent);
    void AdjustUIElement();
};
