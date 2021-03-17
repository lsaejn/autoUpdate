#pragma once
#include <QDialog>
#include <QWidget>
#include <QCheckBox>

/*
模态对话框定制
*/

class QLabel;
class QPushButton;
class QVBoxLayout;
class QCheckBox;

class CustomWidget :
    public QDialog
{
    Q_OBJECT
public:
public:
    CustomWidget(QWidget* parent = 0);
    ~CustomWidget();

protected:
    void showEvent(QShowEvent* event);

    void CenterWindow();
    void CenterOnParent();

    bool m_bPressed = false;
    QWidget* parent_;
    QPoint m_point;
    QVBoxLayout* m_pLayout;
};



class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget* parent = 0);
    ~TitleBar();

protected:

    virtual void mousePressEvent(QMouseEvent* event);
    virtual bool eventFilter(QObject* obj, QEvent* event);

private slots:
    void onClicked();

private:
    QLabel* m_pIconLabel;
    QLabel* m_pTitleLabel;
    QPushButton* m_pCloseButton;
};

#include <QMessageBox>
#include <QDialogButtonBox>
#include <QGridLayout>

class Alime_MessageBox : public CustomWidget
{
    Q_OBJECT

public:
    Alime_MessageBox(QWidget* parent = 0, const QString& title = tr("Tip"), const QString& text = "",
        QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::Ok);
    ~Alime_MessageBox();
    QAbstractButton* clickedButton() const;
    QMessageBox::StandardButton standardButton(QAbstractButton* button) const;
    // 设置默认按钮
    void setDefaultButton(QPushButton* button);
    void setDefaultButton(QMessageBox::StandardButton button);
    // 设置窗体标题
    void setTitle(const QString& title);
    // 设置提示信息
    void setText(const QString& text);
    // 设置窗体图标
    void setIcon(const QString& icon);
    // 添加控件-替换提示信息所在的QLabel
    void addWidget(QWidget* pWidget);

protected:
    // 多语言翻译
    void changeEvent(QEvent* event);

private slots:
    void onButtonClicked(QAbstractButton* button);

private:
    void translateUI();
    int execReturnCode(QAbstractButton* button);

private:
    QLabel* m_pIconLabel;
    QLabel* m_pLabel;
    QGridLayout* m_pGridLayout;
    QDialogButtonBox* m_pButtonBox;
    QAbstractButton* m_pClickedButton;
    QAbstractButton* m_pDefaultButton;
};


static QMessageBox::StandardButton showInformation(QWidget* parent, const QString& title,
    const QString& text, QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton)
{
    Alime_MessageBox msgBox(parent, title, text, buttons, defaultButton);
    msgBox.setIcon(":/Images/information");
    if (msgBox.exec() == -1)
        return QMessageBox::Cancel;
    return msgBox.standardButton(msgBox.clickedButton());
}

static QMessageBox::StandardButton showError(QWidget* parent, const QString& title,
    const QString& text, QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton)
{
    Alime_MessageBox msgBox(parent, title, text, buttons, defaultButton);
    msgBox.setIcon(":/Images/error");
    if (msgBox.exec() == -1)
        return QMessageBox::Cancel;
    return msgBox.standardButton(msgBox.clickedButton());
}

static QMessageBox::StandardButton showSuccess(QWidget* parent, const QString& title,
    const QString& text, QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton)
{
    Alime_MessageBox msgBox(parent, title, text, buttons, defaultButton);
    msgBox.setIcon(":/Images/success");
    if (msgBox.exec() == -1)
        return QMessageBox::Cancel;
    return msgBox.standardButton(msgBox.clickedButton());
}

static QMessageBox::StandardButton showQuestion(QWidget* parent, const QString& title,
    const QString& text, QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton)
{
    Alime_MessageBox msgBox(parent, title, text, buttons, defaultButton);
    msgBox.setIcon(":/Images/question");
    if (msgBox.exec() == -1)
        return QMessageBox::Cancel;
    return msgBox.standardButton(msgBox.clickedButton());
}

static QMessageBox::StandardButton showWarning(QWidget* parent, const QString& title,
    const QString& text, QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton)
{
    Alime_MessageBox msgBox(parent, title, text, buttons, defaultButton);
    msgBox.setIcon(":/images/warning");
    if (msgBox.exec() == -1)
        return QMessageBox::Cancel;
    return msgBox.standardButton(msgBox.clickedButton());
}

static QMessageBox::StandardButton showCritical(QWidget* parent, const QString& title,
    const QString& text, QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton)
{
    Alime_MessageBox msgBox(parent, title, text, buttons, defaultButton);
    msgBox.setIcon(":/Images/warning");
    if (msgBox.exec() == -1)
        return QMessageBox::Cancel;
    return msgBox.standardButton(msgBox.clickedButton());
}

static QMessageBox::StandardButton showCheckBoxQuestion(QWidget* parent, const QString& title,
    const QString& text, QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton)
{
    Alime_MessageBox msgBox(parent, title, text, buttons, defaultButton);
    msgBox.setIcon(":/Images/question");

    QCheckBox* pCheckBox = new QCheckBox(&msgBox);
    pCheckBox->setText(text);
    msgBox.addWidget(pCheckBox);
    if (msgBox.exec() == -1)
        return QMessageBox::Cancel;

    QMessageBox::StandardButton standardButton = msgBox.standardButton(msgBox.clickedButton());
    if (standardButton == QMessageBox::Yes)
    {
        return pCheckBox->isChecked() ? QMessageBox::Yes : QMessageBox::No;
    }
    return QMessageBox::Cancel;
}

