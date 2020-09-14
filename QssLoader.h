#pragma once
#include <QString>
#include <QFile>
#include <QtWidgets/QApplication>

class QssLoader
{
public:
	QssLoader(const QString& name, QApplication& app)
	{
		QFile qssfile(":/qss/dark.qss");
		qssfile.open(QFile::ReadOnly);
		app.setStyleSheet(qssfile.readAll());

		QFont font("Microsoft YaHei");
		font.setPointSize(8);
		app.setFont(font);
	}

};
