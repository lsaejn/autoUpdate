#pragma once
#include <QString>
#include <QFile>
#include <QtWidgets/QApplication>

class QssLoader
{
public:
	QssLoader(const QString& name, QApplication& app, const QString& fontName= "Microsoft YaHei", size_t fontSize=8)
	{
		QFile qssfile(name);
		qssfile.open(QFile::ReadOnly);
		app.setStyleSheet(qssfile.readAll());

		QFont font(fontName);
		font.setPointSize(fontSize);
		app.setFont(font);
	}

};
