#pragma once

#include <QWidget>

class Alime_ContentWidget;

using ContentWidgetCreator=
std::function<Alime_ContentWidget*(QWidget*)>;
 

class Alime_ContentWidget : public QWidget
{
    Q_OBJECT
public:
    Alime_ContentWidget(QWidget* parent = Q_NULLPTR);
    QString GetTitle();
    QString GetIcon();
    int GetShadowWidth();
    QSize GetWindowSize();
    static ContentWidgetCreator creator_;
};


#define CLASSREGISTER(className) \
class Register##className \
{ \
public: \
    Register##className(){ \
         Alime_ContentWidget::creator_ = [](QWidget* parent) { \
             return new className(parent); \
          }; \
    } \
}; Register##className instance_; \
