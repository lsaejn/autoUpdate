#pragma once

#include <QtGlobal>

class QtAlimeStyle;
class QtAlimeTheme;

class QtAlimeStylePrivate
{
    Q_DISABLE_COPY(QtAlimeStylePrivate)
    Q_DECLARE_PUBLIC(QtAlimeStyle)

public:
    QtAlimeStylePrivate::QtAlimeStylePrivate(QtAlimeStyle* q)
        : q_ptr(q)
    {
    }

    ~QtAlimeStylePrivate() = default;

    void init();

    QtAlimeStyle*const q_ptr;
    QtAlimeTheme *theme;
};
