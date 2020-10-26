#pragma once

#include <QVector>
#include <QString>

using PackList=const QVector<QString>&;

class WebFileFilterInterface
{
public:
	virtual PackList FilterFixPackage(PackList list) = 0;
	virtual PackList FilterUpdatePackage(PackList list) = 0;
};
