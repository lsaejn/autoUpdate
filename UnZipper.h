#pragma once

#include "windows.h"

#include <string>

#include <QFile>
#include <QFileInfo>
#include <QtGlobal>
#include <QApplication>

#include "AppUtility.h"
#include "thirdParty/zlib/UnZip.h"


/*
*use like this, if all setting are default
*UnZipper up;
*up.SetResource(src);
*up.UnZip();
*/

class UnZipper
{
	Q_DISABLE_COPY(UnZipper)
public:
	UnZipper() = default;
	~UnZipper()
	{
		CloseZip(hz_);
	}

	int GetFileCount()
	{
		ZIPENTRY ze;
		if(ZR_OK!=GetZipItem(hz_, -1, &ze))
			return -1;
		return ze.index;
	}

	QVector<QString> GetFileList(bool& ret)
	{
		QVector<QString> fileList;
		int numitems = GetFileCount();
		if (numitems < 0)
		{
			ret = false;
			return{};
		}
		ZIPENTRY ze;
		for (int i = 0; i < numitems; i++)
		{
			if (ZR_OK != GetZipItem(hz_, i, &ze))
			{
				ret = false;
				return {};
			}
			else
			{
				//效率何在0.0
				std::wstring wstr = ze.name;
				QString qstr = QString::fromStdWString(wstr);
				qstr = qstr.mid(qstr.indexOf(L'/') + 1);
				if (qstr.isEmpty())
					continue;
				fileList.push_back(qstr);
			}
		}
		ret = true;
		return fileList;
	}

	bool SetResource(const std::wstring& path)
	{
		srcPath_ = path;
		hz_ = OpenZip(srcPath_.c_str(), nullptr);
		return hz_;
	}

	void SetTargetPath(std::wstring&& path)
	{
		if (path.empty())
			desPath_ = GetPkpmRootPath().toStdWString();
		desPath_ = std::move(path);
		if (desPath_.back() != L'\\' && desPath_.back() != L'/')
			desPath_ += L"/";
	}

	bool Backup()
	{
		QDir dir{QString::fromStdWString(backupPath_)};
		if (dir.exists())
		{
			bool ret=dir.removeRecursively();
			if (!ret)
			{
				//fix me
				return false;
			}
			ret=dir.mkpath(QString::fromStdWString(backupPath_));
			return BackupTo(backupPath_);
		}
		else
		{
			bool ret=dir.mkpath(QString::fromStdWString(backupPath_));
			if (!ret)
				return false;
			return BackupTo(backupPath_);
		}
	}

	bool UnZip()
	{
		return UnZipTo(desPath_);
	}

	//fix me, 依赖顺序的
	void SetBackupRootPath(const std::wstring& path)
	{
		if (path.empty())
		{
			backupPath_ = GetExeFolderW()+L"backup/"+GetPackName()+L"/";
		}
		else
		{
			backupPath_ = path;
		}
		for (size_t i = 0; i != backupPath_.size(); ++i)
		{
			if (backupPath_[i] == L'\\')
				backupPath_[i] = L'/';
		}
	}

	bool Recover()
	{
		return RecoverFrom(backupPath_);
	}

private:
	//备份原始文件到一个目录
	//path root/installer/backup/{packname}
	bool BackupTo(const std::wstring& path)
	{
		bool ret=true;
		auto files=GetFileList(ret);
		if (!ret)
			return false;
		QString root= QString::fromStdWString(desPath_);
		for (int i=0; i!=files.size(); ++i)
		{
			const QString& file = files[i];
			if (file.endsWith("/"))
			{
				QDir dir{ QString::fromStdWString(path) + file };
				if (!dir.exists())
					dir.mkdir(QString::fromStdWString(path) + file);
				continue;
			}
			QFile f(root+file);
			if (!f.exists())
			{
				qDebug() << "a new file, so we skip backup" << file;
			}
			else
			{
				QString targetFile= QString::fromStdWString(path)+file;
				auto copyRet=f.copy(targetFile);
				if (!copyRet)
				{
					qDebug() << "failed to backup file: " << file;
					ret = false;
				}
				else
					qDebug() << "backup file: " << file;
			}
		}
		return ret;
	}

	//fix me, 没有时间，只好搞豆腐渣工程了
	bool	RecoverFrom(const std::wstring& path)
	{
		return true;
	}

	bool UnZipTo(const std::wstring& path)
	{
		if (!Backup())
			return false;
		bool ret = true;
		auto files = GetFileList(ret);
		for (int i = 0; i != files.size(); ++i)
		{
			const QString& file = files[i];
			if (file.endsWith("/"))
			{
				QDir dir{ QString::fromStdWString(path) + file };
				if (!dir.exists())
					dir.mkdir(QString::fromStdWString(path) + file);
				continue;
			}
		}
		int numitems = GetFileCount();
		ZIPENTRY ze;
		for (int i = 0; i < numitems; i++)
		{
			if (ZR_OK != GetZipItem(hz_, i, &ze))
			{
				qDebug() << "GetZipItem error when trying to unzip to local path";
				return false;
			}
			if (ze.attr & FILE_ATTRIBUTE_DIRECTORY)//skip dir
			{
				continue;
			}
			else
			{
				std::wstring surfix = ze.name;
				surfix = surfix.substr(surfix.find(L"/") + 1);
				std::wstring filaname = path + surfix;//ze not begin with seprator
				auto ret = UnzipItem(hz_, i, filaname.c_str());
				if (ZR_OK != ret)
					return false;
			}
		}
		return true;
	}

	std::wstring GetPackName()
	{
		int index = -1;
		for (int i = srcPath_.size(); i >= 0; i--)
		{
			if (srcPath_[i] == L'/' || srcPath_[i] == L'\\')
			{
				index = i;
				break;
			}
		}
		return srcPath_.substr(index + 1);
	}

private:
	std::wstring srcPath_;//zip file
	std::wstring desPath_;//folder
	std::wstring backupPath_;//folder
	HZIP hz_;
};

