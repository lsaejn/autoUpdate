#pragma once

#include <QtWidgets/QWidget>
#include <QLayout>

class Alime_TitleBar;

class Alime_WindowBase : public QWidget
{
    Q_OBJECT
public:
    Alime_WindowBase(QWidget* parent = Q_NULLPTR, QLayout* ownerBox= Q_NULLPTR);
private:
    Alime_TitleBar* titleBar_;
    QLayout* box_;
};
