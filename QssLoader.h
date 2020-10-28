#pragma once
#include <QString>
#include <QFile>
#include <QtWidgets/QApplication>
#include <QFileSystemWatcher>
#include <QObject>
#include <QFont>

#include "AppUtility.h"

class QssLoader: public QObject
{
	Q_OBJECT
public:
	QssLoader(const QString& name, QApplication& app, const QString& fontName= "Microsoft YaHei", size_t fontSize=8)
	{
		QFile qssfile(name);
		qssfile.open(QFile::ReadOnly);
		app.setStyleSheet(qssfile.readAll());

		QFont font(fontName);
		font.setPointSize(fontSize);
		app.setFont(font);

		watcher_ = new QFileSystemWatcher(this);

		watcher_->addPath(GetStyleFilePath());
		connect(watcher_, &QFileSystemWatcher::fileChanged, [this]() {
			auto style=GetStyleName();
			SetStyle(style);
			});
	}
private:
	void SetStyle(const QString& name)
	{
		QFile qssfile(name);
		qssfile.open(QFile::ReadOnly);
		qApp->setStyleSheet(qssfile.readAll());
	}

private:
	QFileSystemWatcher *watcher_;
};
