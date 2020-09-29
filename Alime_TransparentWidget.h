#pragma once
#include <QWidget>
/*
--------------Alime_TransparentWidget---
|-------------Alime_WindowBase----------|
|-------------Alime_TitleBar---------------.|
|          |                                                    |
|          |------Alime_ContentWidget-------|
|          |                                                    |
|----------------------------------------......|
*/

class Alime_TransparentWidget : public QWidget
{
    Q_OBJECT

public:
    Alime_TransparentWidget(QWidget* parent = Q_NULLPTR);
    virtual bool nativeEvent(const QByteArray& eventType, void* message, long* result);
private:
    int boundaryWidth;
};
