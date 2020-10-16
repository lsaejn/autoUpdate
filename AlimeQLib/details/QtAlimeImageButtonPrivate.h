#include <QtGlobal>
#include <QColor>

class QtAlimeImageButton;
class QtAlimeRippleOverlay;

/*
��������Button��Ripple�ĸ�����
*/
class QtAlimeImageButtonPrivate
{
    Q_DISABLE_COPY(QtAlimeImageButtonPrivate)
    Q_DECLARE_PUBLIC(QtAlimeImageButton)

public:
    QtAlimeImageButtonPrivate(QtAlimeImageButton*q);
    virtual ~QtAlimeImageButtonPrivate();

    void init();
    void updateRipple();

    QColor color;
    QColor disabledColor;
    bool useThemeColors;

    QtAlimeImageButton* const q_ptr;
    QtAlimeRippleOverlay* rippleOverlay;
};
