#pragma once
/*
RippleHelp, use like this:

Class Foo: public QtAlimeRippleHolder
{
}

Foo f;
f.AttachRippleEvent();
这样, 既有类只需要继承一个基类即可。
没有什么好办法
*/
#include <QVector>
class QtAlimeRippleOverlay;

class QtAlimeRippleHolder
{
public:
	QtAlimeRippleHolder()
	{

	}

	virtual ~QtAlimeRippleHolder()
	{

	}

	void AttachRippleEvent()
	{

	}


private:
	QVector<QtAlimeRippleOverlay*> rippleOverlays_;
};

