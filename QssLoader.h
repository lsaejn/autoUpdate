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
		name_ = name;
		app_ = &app;
		QFile qssfile(name_);
		qssfile.open(QFile::ReadOnly);
		app.setStyleSheet(qssfile.readAll());

		QFont font(fontName);
		font.setPointSize(fontSize);
		app.setFont(font);

		//什么Bug?
		watcher_ = new QFileSystemWatcher(this);
		watcher_->addPath(GetStyleFilePath());
		connect(watcher_, &QFileSystemWatcher::fileChanged, this, &QssLoader::SetStyle, Qt::QueuedConnection);
	}
private slots:
	void SetStyle()
	{
		auto name = GetStyleName();
		if (name == name_)
			return;
		QFile qssfile(name);
		qssfile.open(QFile::ReadOnly);
		auto str = qssfile.readAll();
		//Qt的bug，和ListWidget一样,解决办法就是设为空，再读一遍数据
		app_->setStyleSheet("");
		app_->setStyleSheet(str);
		name_ = name;
	}

private:
	QFileSystemWatcher *watcher_;
	QString name_;
	QApplication* app_;
};


