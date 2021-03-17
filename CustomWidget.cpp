#include "CustomWidget.h"


#include <QDesktopWidget>
#include <QEvent>
#include <QMouseEvent>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QApplication>
#include <QCheckBox>

#ifdef Q_OS_WIN
#pragma comment(lib, "user32.lib")
#include <qt_windows.h>
#endif

CustomWidget::CustomWidget(QWidget* parent) :
    QDialog(nullptr),
    parent_(parent)
{
    resize(360, 200);
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    setAttribute(Qt::WA_StyledBackground, true);

    TitleBar* pTitleBar = new TitleBar(this);
    installEventFilter(pTitleBar);

    setWindowTitle("Custom Window");
    setObjectName("CustomWidget");
    //setWindowIcon(QIcon(":/Images/logo"));

    m_pLayout = new QVBoxLayout(this);
    m_pLayout->addWidget(pTitleBar);
    m_pLayout->addStretch();
    m_pLayout->setSpacing(0);
    m_pLayout->setContentsMargins(0, 0, 0, 0);
}

CustomWidget::~CustomWidget()
{

}

void CustomWidget::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);

    CenterWindow();
}

void CustomWidget::CenterWindow()
{
    if (parent_)
    {
        CenterOnParent();
        return;
    }
        
    QDesktopWidget screen;
    QRect screenGeom = screen.screenGeometry(this);
    int screenCenterX = screenGeom.center().x();
    int screenCenterY = screenGeom.center().y();
    move(screenCenterX - width() / 2, screenCenterY - height() / 2);
}


void CustomWidget::CenterOnParent()
{
    QRect geom=parent_->window()->geometry();
    int x = geom.x()+(geom.width()- width())/2;
    int y = geom.y()+ (geom.height() - height()) / 2;

    move(x, y);
}




TitleBar::TitleBar(QWidget* parent)
    : QWidget(parent)
{
    setFixedHeight(30);
    setObjectName("DialogTitle");
    setAttribute(Qt::WA_StyledBackground, true);
    m_pIconLabel = new QLabel(this);
    m_pTitleLabel = new QLabel(this);

    m_pCloseButton = new QPushButton(this);

    m_pIconLabel->setFixedSize(20, 20);
    m_pIconLabel->setScaledContents(true);

    m_pTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_pCloseButton->setFixedSize(27, 22);
    m_pCloseButton->setIcon(QIcon(":/images/close.png"));

    m_pTitleLabel->setObjectName("DialogTileLabel");

    QHBoxLayout* pLayout = new QHBoxLayout(this);
    pLayout->addWidget(m_pIconLabel);
    pLayout->addSpacing(5);
    pLayout->addWidget(m_pTitleLabel);
    pLayout->addWidget(m_pCloseButton);
    pLayout->setSpacing(0);
    pLayout->setContentsMargins(5, 0, 5, 0);

    setLayout(pLayout);

    connect(m_pCloseButton, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
}

TitleBar::~TitleBar()
{

}


void TitleBar::mousePressEvent(QMouseEvent* event)
{
#ifdef Q_OS_WIN
    if (ReleaseCapture())
    {
        QWidget* pWindow = this->window();
        if (pWindow->isTopLevel())
        {
            SendMessage(HWND(pWindow->winId()), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
        }
    }
    event->ignore();
#else
#endif
}

bool TitleBar::eventFilter(QObject* obj, QEvent* event)
{
    switch (event->type())
    {
    case QEvent::WindowTitleChange:
    {
        QWidget* pWidget = qobject_cast<QWidget*>(obj);
        if (pWidget)
        {
            m_pTitleLabel->setText(pWidget->windowTitle());
            return true;
        }
    }
    case QEvent::WindowIconChange:
    {
        QWidget* pWidget = qobject_cast<QWidget*>(obj);
        if (pWidget)
        {
            QIcon icon = pWidget->windowIcon();
            m_pIconLabel->setPixmap(icon.pixmap(m_pIconLabel->size()));
            return true;
        }
    }
    }
    return QWidget::eventFilter(obj, event);
}

void TitleBar::onClicked()
{
    QPushButton* pButton = qobject_cast<QPushButton*>(sender());
    QWidget* pWindow = this->window();
    if (pWindow->isTopLevel())
    {
        if (pButton == m_pCloseButton)
        {
            pWindow->close();
        }
    }
}



#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QEvent>
#include <QApplication>

Alime_MessageBox::Alime_MessageBox(QWidget* parent, const QString& title, const QString& text,
    QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
    : CustomWidget(parent)
{
    setWindowIcon(QIcon(":/Images/logo"));
    setWindowTitle(title);
    setMinimumSize(300, 130);

    m_pButtonBox = new QDialogButtonBox(this);
    m_pButtonBox->setStandardButtons(QDialogButtonBox::StandardButtons(int(buttons)));
    setDefaultButton(defaultButton);

    QPushButton* pYesButton = m_pButtonBox->button(QDialogButtonBox::Yes);
    if (pYesButton != NULL)
    {
        pYesButton->setObjectName("blueButton");
        pYesButton->setStyle(QApplication::style());
    }

    m_pIconLabel = new QLabel(this);
    m_pLabel = new QLabel(this);

    QPixmap pixmap(":/Images/information");
    m_pIconLabel->setPixmap(pixmap);
    m_pIconLabel->setFixedSize(35, 35);
    m_pIconLabel->setScaledContents(true);

    m_pLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_pLabel->setObjectName("whiteLabel");
    m_pLabel->setOpenExternalLinks(true);
    m_pLabel->setText(text);

    m_pGridLayout = new QGridLayout();
    m_pGridLayout->addWidget(m_pIconLabel, 0, 0, 2, 1, Qt::AlignTop);
    m_pGridLayout->addWidget(m_pLabel, 0, 1, 2, 1);
    m_pGridLayout->addWidget(m_pButtonBox, m_pGridLayout->rowCount(), 0, 1, m_pGridLayout->columnCount());
    m_pGridLayout->setSizeConstraint(QLayout::SetNoConstraint);
    m_pGridLayout->setHorizontalSpacing(10);
    m_pGridLayout->setVerticalSpacing(10);
    m_pGridLayout->setContentsMargins(10, 10, 10, 10);
    m_pLayout->addLayout(m_pGridLayout);

    translateUI();

    connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButtonClicked(QAbstractButton*)));
}

Alime_MessageBox::~Alime_MessageBox()
{

}

void Alime_MessageBox::changeEvent(QEvent* event)
{
    switch (event->type())
    {
    case QEvent::LanguageChange:
        translateUI();
        break;
    default:
        CustomWidget::changeEvent(event);
    }
}

void Alime_MessageBox::translateUI()
{
    QPushButton* pYesButton = m_pButtonBox->button(QDialogButtonBox::Yes);
    if (pYesButton != NULL)
        pYesButton->setText(tr("Yes"));

    QPushButton* pNoButton = m_pButtonBox->button(QDialogButtonBox::No);
    if (pNoButton != NULL)
        pNoButton->setText(tr("No"));

    QPushButton* pOkButton = m_pButtonBox->button(QDialogButtonBox::Ok);
    if (pOkButton != NULL)
        pOkButton->setText(tr("Ok"));

    QPushButton* pCancelButton = m_pButtonBox->button(QDialogButtonBox::Cancel);
    if (pCancelButton != NULL)
        pCancelButton->setText(tr("Cancel"));
}

QMessageBox::StandardButton Alime_MessageBox::standardButton(QAbstractButton* button) const
{
    return (QMessageBox::StandardButton)m_pButtonBox->standardButton(button);
}

QAbstractButton* Alime_MessageBox::clickedButton() const
{
    return m_pClickedButton;
}

int Alime_MessageBox::execReturnCode(QAbstractButton* button)
{
    int nResult = m_pButtonBox->standardButton(button);
    return nResult;
}

void Alime_MessageBox::onButtonClicked(QAbstractButton* button)
{
    m_pClickedButton = button;
    done(execReturnCode(button));
}

void Alime_MessageBox::setDefaultButton(QPushButton* button)
{
    if (!m_pButtonBox->buttons().contains(button))
        return;
    m_pDefaultButton = button;
    button->setDefault(true);
    button->setFocus();
}

void Alime_MessageBox::setDefaultButton(QMessageBox::StandardButton button)
{
    setDefaultButton(m_pButtonBox->button(QDialogButtonBox::StandardButton(button)));
}

void Alime_MessageBox::setTitle(const QString& title)
{
    setWindowTitle(title);
}

void Alime_MessageBox::setText(const QString& text)
{
    m_pLabel->setText(text);
}

void Alime_MessageBox::setIcon(const QString& icon)
{
    m_pIconLabel->setPixmap(QPixmap(icon));
}

void Alime_MessageBox::addWidget(QWidget* pWidget)
{
    m_pLabel->hide();
    m_pGridLayout->addWidget(pWidget, 0, 1, 2, 1);
}


