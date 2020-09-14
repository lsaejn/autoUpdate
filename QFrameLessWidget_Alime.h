#pragma once

#include <QtWidgets/QWidget>
#include "Alime_ContentWidget.h"

class DownloadInfoWidget;
class QListWidget;
class QFrameLessWidget_Alime : public Alime_ContentWidget
{
    Q_OBJECT

public:
    QFrameLessWidget_Alime(QWidget *parent = Q_NULLPTR);

private:
    bool InitDownloadList();
    QListWidget* downloadList_;
    QWidget* leftContent_;
    QWidget* rightContent_;
};


class DownloadInfoWidget : public QWidget
{
    Q_OBJECT

public:
    DownloadInfoWidget(QWidget* parent = Q_NULLPTR);
    ~DownloadInfoWidget();
};