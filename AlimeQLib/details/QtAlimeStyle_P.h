#pragma once

#include <QtGlobal>

class QtAlimeStyle;
class QtAlimeTheme;

class QtAlimeStylePrivate
{
    Q_DISABLE_COPY(QtAlimeStylePrivate)
    Q_DECLARE_PUBLIC(QtAlimeStyle)

public:
    QtAlimeStylePrivate(QtAlimeStyle*q);
    ~QtAlimeStylePrivate();

    void init();

    QtAlimeStyle*const q_ptr;
    QtAlimeTheme *theme;
};
