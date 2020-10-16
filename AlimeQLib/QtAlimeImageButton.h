#pragma once

#include <QtWidgets/QAbstractButton>

class QtAlimeImageButtonPrivate;

/*
* 关于Q_D宏。这个是套路写法，知道就行了
https://www.cnblogs.com/microthink/p/9281848.html
*/

class QtAlimeImageButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit  QtAlimeImageButton(const QIcon& icon, QWidget* parent = 0);
    ~QtAlimeImageButton();

    QSize sizeHint() const Q_DECL_OVERRIDE;

    void setUseThemeColors(bool value);
    bool useThemeColors() const;

    void setColor(const QColor& color);
    QColor color() const;

    void setDisabledColor(const QColor& color);
    QColor disabledColor() const;

protected:
    QtAlimeImageButton(QtAlimeImageButtonPrivate& d, QWidget* parent = 0);

    bool event(QEvent* event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject* obj, QEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

    const QScopedPointer<QtAlimeImageButtonPrivate> d_ptr;

private:
    Q_DISABLE_COPY(QtAlimeImageButton)
    Q_DECLARE_PRIVATE(QtAlimeImageButton)
};