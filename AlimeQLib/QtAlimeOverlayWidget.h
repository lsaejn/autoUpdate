#pragma once

#include <QtWidgets/QWidget>

/*
一个用于在parent上层显示的Widget
*/
class QtAlimeOverlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QtAlimeOverlayWidget(QWidget *parent = 0);
    ~QtAlimeOverlayWidget();

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

    virtual QRect overlayGeometry() const;

private:
    Q_DISABLE_COPY(QtAlimeOverlayWidget)
};
