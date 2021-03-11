#include "Alime_WindowBase.h"
#include "Alime_TitleBar.h"
#include "Alime_ContentWidget.h"

#include <qpushbutton.h>

#include "qlayout.h"
#include "qicon.h"

#ifdef Q_OS_WIN
#pragma comment(lib, "user32.lib")
#include <qt_windows.h>
#endif

Alime_WindowBase::Alime_WindowBase(QWidget* parent, QLayout* ownerBox)
    : QWidget(parent),
    titleBar_(nullptr),
    box_(ownerBox)
{
    //¿Í»§Çø
    auto content = Alime_ContentWidget::creator_(this);
    titleBar_ = new Alime_TitleBar(this);

    //fix me£¬modify shadowWindow or remove installEventFilter
    installEventFilter(titleBar_);

    box_->setMargin(content->GetShadowWidth());
    titleBar_->SysButtonEventRegister([=]() { box_->setMargin(content->GetShadowWidth());}, false);
    titleBar_->SysButtonEventRegister([=]() {box_->setMargin(0);}, true);

    setWindowTitle(content->GetTitle());
    setWindowIcon(QIcon(content->GetIcon()));
    parentWidget()->resize(content->GetWindowSize());

    QPalette pal(palette());
    pal.setColor(QPalette::Background, QColor(255,255,255));
    setAutoFillBackground(true);
    setPalette(pal);

    QVBoxLayout* pLayout = new QVBoxLayout();
    pLayout->addWidget(titleBar_);
    pLayout->addWidget(content);

    pLayout->setSpacing(0);
    pLayout->setMargin(0);
    setLayout(pLayout);
}


