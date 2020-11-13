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

		//ʲôBug?
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
		//Qt��bug����ListWidgetһ��,����취������Ϊ�գ��ٶ�һ������
		app_->setStyleSheet("");
		app_->setStyleSheet(str);
		name_ = name;
	}

private:
	QFileSystemWatcher *watcher_;
	QString name_;
	QApplication* app_;
};


