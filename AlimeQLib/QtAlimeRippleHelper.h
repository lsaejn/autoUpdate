#pragma once
/*
RippleHelp, use like this:

Class Foo: public QtAlimeRippleHolder
{
}

Foo f;
f.AttachRippleEvent();
����, ������ֻ��Ҫ�̳�һ�����༴�ɡ�
û��ʲô�ð취
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

