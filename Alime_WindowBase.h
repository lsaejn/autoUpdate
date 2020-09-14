#pragma once

#include <QtWidgets/QWidget>
#include <QLayout>

class Alime_TitleBar;
class Alime_WindowBase : public QWidget
{
    Q_OBJECT

public:
    Alime_WindowBase(QWidget* parent = Q_NULLPTR, QLayout* ownerBox= Q_NULLPTR);
    void SetWindowTitle(const QString& titleName);
    void SetWindowIcon(const QString& qrcPath);
    void SetSysButton(const QString& minB, const QString& max, const QString& restore, const QString& close);
private:
    Alime_TitleBar* titleBar_;
    QLayout* box_;
};
